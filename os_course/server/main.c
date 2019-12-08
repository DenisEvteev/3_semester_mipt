#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <errno.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <assert.h>
#include <math.h>

#define NORETURN __attribute__((noreturn))

#define max(a, b) ((a) > (b) ? (a) : (b))

#define error(str)                                              \
do{                                                              \
    fprintf(stderr, "errno == %d\n", errno);                      \
    fprintf(stderr, "line == %d\n", __LINE__);                     \
    fprintf(stderr, "function_name == %s\n", __PRETTY_FUNCTION__);  \
    perror(str);                                                     \
    exit(EXIT_FAILURE);                                               \
}while(0)                                                              \


#define c_c(fd)                                \
do{                                             \
    int res = close(fd);                         \
    if(res == -1)                                 \
        error("error in close");                   \
}while(0)                                           \


#define o_non(fd)                                 \
    do{                                            \
        int flags = fcntl(fd, F_GETFL);             \
        if(flags == -1)                              \
            error("error fcntl F_GETFL");             \
        flags |= O_NONBLOCK;                           \
        if(fcntl(fd, F_SETFL, flags) == -1)             \
            error("error fcntl F_SETFL");                \
                                                          \
    }while(0)                                              \


typedef struct link_t {
    int p_in[2];
    int p_out[2];

    char *buf;
    //the number of bytes that actually busy in the buf
    ssize_t size_;
    /*This field is responsible for writing the data to pipe starting with the correct address*/
    size_t off_wr;

    size_t cap_;
    //this field for syscall select -- nfds = cur_max_fd + 1
    int cur_max_fd;
} link_t;

enum
{
    BASE = 10,
    EMPTY_FD = -1,
    SECONDS = 5,
    M_SECONDS = 0
};

enum SIZE {
    CHILD_BUF_SIZE = 128000,
    BUF_SIZE = 1024 // min capacity for the parent buffer
};

void
child(int fd_read, int fd_write) NORETURN;

void
host(link_t *connections, unsigned n);

size_t
buf_size(unsigned pow_i);

link_t *create_array_pipes(size_t n);

int
main(int argc, char *argv[]) {
    if (argc != 3) {
        errno = E2BIG;
        error("bad number of arguments");
    }


    long n = strtol(argv[1], NULL, BASE);
    if ((errno == ERANGE && (n == LONG_MAX || n == LONG_MIN)) ||
        (errno != 0 && n == 0) || n <= 0) {
        fprintf(stderr, "number of children == %s\n", argv[1]);
        error("bad number of children");
    }

    int fd = open(argv[2], O_RDONLY);
    if (fd == -1)
        error("error in opening file with data");

    //create all the pipes in advance in parent process
    link_t *connections = create_array_pipes(n);

    pid_t pid = 0;
    int i;
    for (i = 0; i < n; ++i) {
        pid = fork();

        /*Child close descriptors that it won't use at all*/
        if (pid == 0) {
            if (i != 0)
                c_c(connections[i - 1].p_out[1]);
            c_c(connections[i].p_in[0]);
            break;
        }

        else if (pid > 0)
        {
            c_c(connections[i].p_in[1]);

            connections[i].cur_max_fd = max(connections[i].p_in[0], connections[i].p_out[1]);
            if (i != 0 && connections[i].cur_max_fd < connections[i - 1].cur_max_fd)
                connections[i].cur_max_fd = connections[i - 1].cur_max_fd;
        }

        if (pid == -1)
            error("error in fork");
    }

    /*The continue of the child process*/
    if (!pid)
        i ? child(connections[i - 1].p_out[0], connections[i].p_in[1]) : child(fd, connections[i].p_in[1]);

    else {
        /*close the rest part of the descriptors in the parent which will be useless in it*/
        for (int i = 0; i < n; ++i) {
            if (i != n - 1)
                c_c(connections[i].p_out[0]);
        }

        host(connections, n);
    }

    return 0;
}

link_t *create_array_pipes(size_t n) {
    assert(n > 0);
    link_t *connections = calloc(n, sizeof(link_t));
    assert(connections);

    for (int i = 0; i < n; ++i) {
        if (pipe(connections[i].p_in) == -1)
            error("error in pipe");
        if (i == n - 1) {
            connections[i].p_out[1] = STDOUT_FILENO;
            connections[i].p_out[0] = EMPTY_FD;
        } else {
            if (pipe(connections[i].p_out) == -1)
                error("error in pipe");
        }
    }
    return connections;
}

void
host(link_t *connections, unsigned n) {
    assert(connections && n > 0);
    fd_set rd, wr;
    struct timeval timeout;
    timeout.tv_sec = SECONDS;
    timeout.tv_usec = M_SECONDS;


    for (int i = 0; i < n; ++i) {
        o_non(connections[i].p_in[0]);
        if (i != n - 1)
            o_non(connections[i].p_out[1]);
        connections[i].cap_ = buf_size(n - i);
        connections[i].size_ = 0;
        connections[i].off_wr = 0;
        connections[i].buf = calloc(connections[i].cap_, sizeof(char));
        assert(connections[i].buf);
    }

    /*This variables are used to determine when I must go out from the
     * endless loop --- I mean that the case of their equality we consider to be the point when
     * we should leave the loop*/
    off_t into = 0;
    off_t from = 0;

    //int ip_ = 0;
    for (;;) {
        FD_ZERO(&rd);
        FD_ZERO(&wr);
//
//        if(ip_ == n)
//            --ip_;

        for (int i = 0; i < n; ++i) {
            FD_SET(connections[i].p_in[0], &rd);
            FD_SET(connections[i].p_out[1], &wr);
        }

        int number_fd = select(connections[n - 1].cur_max_fd + 1, &rd, &wr, NULL, &timeout);
        if (number_fd == -1)
            error("error in select");

        int finished_operations = 0;
        int counter = 0;
        ssize_t write_b = 0;
        while (finished_operations != number_fd)
        {
            if (FD_ISSET(connections[counter].p_in[0], &rd)) {
                /*My host won't write to the host buffer until the size of it becomes 0 */
                if (connections[counter].size_ == 0) {
                    connections[counter].size_ = read(connections[counter].p_in[0], connections[counter].buf,
                                                      connections[counter].cap_);
                    if (connections[counter].size_ == -1)
                        error("read error");
                    if (counter == 0)
                        into += connections[counter].size_;
                }

                ++finished_operations;
            }

            if (FD_ISSET(connections[counter].p_out[1], &wr)) {
                if (connections[counter].size_ != 0) {
                    /*Here we write to pipe so we want the bad behaviour of write with o_NONBLOCK
                     * When I've understood that it will be very nice to write BUF_SIZE bytes in the ready for writing descriptor
                     * I've was very glad to write this comment and the code further !!!
                     * Because I've understood the relationship between the ready for writing descriptor
                     * and the current number of bytes in the pipe@@@
                     * It's so tasty and beautiful*/
                    write_b = write(connections[counter].p_out[1],
                                    connections[counter].buf + connections[counter].off_wr,
                                    BUF_SIZE > connections[counter].size_ ? connections[counter].size_
                                                                          : BUF_SIZE); //BUF_SIZE == 1024
                    if (write_b == -1)
                        error("error in write");
                    connections[counter].off_wr += write_b;
                    connections[counter].size_ -= write_b;
                    if (connections[counter].size_ == 0)
                        connections[counter].off_wr = 0;

                    if (counter == n - 1)
                        from += write_b;
                }
                ++finished_operations;

            }

            ++counter;
        }

        if (from && from == into && connections[counter - 1].size_ == 0 && write_b == 0)
            break;
        //++ip_;
    }

    for (int j = 0; j < n; ++j)
    {
        c_c(connections[j].p_in[0]);
        c_c(connections[j].p_out[1]);
        free(connections[j].buf);
    }

    free(connections);
}

void
child(int fd_read, int fd_write) {
    assert(fd_read >= 0 && fd_write >= 0);
    char buf[PIPE_BUF] = {};

    ssize_t bytes_r = 0;
    ssize_t bytes_w = 0;

    while ((bytes_r = read(fd_read, buf, PIPE_BUF)) > 0) {
        if ((bytes_w = write(fd_write, buf, bytes_r)) == -1)
            error("error write");

        assert(bytes_r == bytes_w);
    }
    if (bytes_r == -1)
        error("error read");


    c_c(fd_read);
    c_c(fd_write);
    exit(EXIT_SUCCESS);
}

size_t
buf_size(unsigned pow_i)
{
    errno = 0;
    double size = pow(3, (double) (pow_i));
    if (errno != 0)
        error("error in pow");

    if (size > (double) CHILD_BUF_SIZE)
        return CHILD_BUF_SIZE;

    return (size_t) size * 1024;

}
