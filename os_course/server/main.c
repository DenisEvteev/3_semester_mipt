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

enum {
    BASE = 10,
    EMPTY_FD = -1,
    INITIAL_VALUE = -1
};

enum SIZE {
    CHILD_BUF_SIZE = 128000,
    BUF_SIZE = 1024
};

int
child(int n, int fd_read, int fd_write);

int
host(link_t *connections, unsigned n);

size_t
buf_size(unsigned pow_i);

void close_low_descpts(unsigned up_limit, link_t *connections);


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

    link_t *connections = calloc(n, sizeof(link_t));
    assert(connections);

    pid_t pid = 0;
    int i;
    for (i = 0; i < n; ++i) {
        if (pipe(connections[i].p_in) == -1)
            error("error in pipe");

        pid = fork();

        if (pid == 0) {
            close_low_descpts(i, connections);
            c_c(connections[i].p_in[0]);
            break;
        } else if (pid > 0) {
            if (!i)
                c_c(fd);

            c_c(connections[i].p_in[1]);

            if (i == n - 1) {
                connections[i].p_out[1] = STDOUT_FILENO;
                connections[i].p_out[0] = EMPTY_FD;

            } else {
                if (pipe(connections[i].p_out) == -1)
                    error("error in pipe");
            }

            connections[i].cur_max_fd = max(connections[i].p_in[0], connections[i].p_out[1]);
            if (i != 0 && connections[i].cur_max_fd < connections[i - 1].cur_max_fd)
                connections[i].cur_max_fd = connections[i - 1].cur_max_fd;

        }

        if (pid == -1)
            error("error in fork");
    }

    //children
    if (!pid) {
        i ? child(i, connections[i - 1].p_out[0], connections[i].p_in[1]) : child(i, fd, connections[i].p_in[1]);
    } else {
        for (int i = 0; i < n; ++i) {
            if (i != n - 1)
                c_c(connections[i].p_out[0]);
        }

        host(connections, n);
    }

    free(connections);
    return 0;
}

void close_low_descpts(unsigned up_limit, link_t *connections) {
    assert(up_limit >= 0 && connections);

    for (int i = 0; i < up_limit; ++i) {
        c_c(connections[i].p_in[0]);
        c_c(connections[i].p_out[1]);
        if (i != up_limit - 1)
            c_c(connections[i].p_out[0]);
    }
}

int
host(link_t *connections, unsigned n) {
    assert(connections && n > 0);

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

    int number_process_to_die = INITIAL_VALUE;
    int is_dead = 0;

    ssize_t write_b = 0;
    fd_set rd, wr;
    int n_acts = 0;

    for (;;) {
        FD_ZERO(&rd);
        FD_ZERO(&wr);

        n_acts = 0;

        for (int i = 0; i < n; ++i) {
            if (connections[i].size_ == 0 && i > number_process_to_die) {
                FD_SET(connections[i].p_in[0], &rd);
                ++n_acts;
            }

            if (connections[i].size_ != 0 || (i == number_process_to_die && !is_dead)) {
                FD_SET(connections[i].p_out[1], &wr);
                ++n_acts;
            }
        }

        if (n_acts == 0) break;

        int number_fd = select(connections[n - 1].cur_max_fd + 1, &rd, &wr, NULL, NULL);
        if (number_fd == -1)
            error("error in select");

        int finished_operations = 0;
        int counter = 0;

        while (finished_operations != number_fd) {

            if (FD_ISSET(connections[counter].p_in[0], &rd)) {

                connections[counter].size_ = read(connections[counter].p_in[0], connections[counter].buf,
                                                  connections[counter].cap_);

                if (connections[counter].size_ == -1)
                    error("read error");

                if (connections[counter].size_ == 0) {
                    c_c(connections[counter].p_in[0]);
                    number_process_to_die = counter;
                    is_dead = 0;
                }
                ++finished_operations;
            }

            if (FD_ISSET(connections[counter].p_out[1], &wr)) {

                write_b = write(connections[counter].p_out[1],
                                connections[counter].buf + connections[counter].off_wr,
                                BUF_SIZE > connections[counter].size_ ? connections[counter].size_
                                                                      : BUF_SIZE); //BUF_SIZE == 1024
                if (write_b == -1)
                    error("error in write");

                connections[counter].off_wr += write_b;
                connections[counter].size_ -= write_b;

                if (connections[counter].size_ == 0) {
                    if (counter == number_process_to_die) {
                        if (counter != n - 1)
                            c_c(connections[counter].p_out[1]);
                        is_dead = 1;
                    } else
                        connections[counter].off_wr = 0;
                }
                ++finished_operations;
            }

            ++counter;
        }

    }

    for (int j = 0; j < n; ++j)
        free(connections[j].buf);

    return 0;
}


int
child(int n, int fd_read, int fd_write) {
    assert(fd_read >= 0 && fd_write >= 0 && n >= 0);
    char *buf = calloc(PIPE_BUF, sizeof(char));

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
    free(buf);
    return 0;
}

size_t
buf_size(unsigned pow_i) {
    errno = 0;
    double size = pow(3, (double) (pow_i));
    if (errno != 0)
        error("error in pow");

    if (size > (double) CHILD_BUF_SIZE)
        return CHILD_BUF_SIZE;

    return (size_t) size * 1024;

}
