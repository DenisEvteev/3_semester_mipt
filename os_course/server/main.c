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


struct link_t {
    int p_in[2];
    int p_out[2];

    char* buf;
    size_t size_;
    //this field for syscall select -- nfds = cur_max_fd + 1
    int cur_max_fd;
};


enum {
    BASE = 10,
    EMPTY_FD = -1,
    SECONDS = 10,
    M_SECONDS = 0
};

enum SIZE{
    CHILD_BUF_SIZE = 128000
};

void child(int fd_read, int fd_write) NORETURN;

void host(struct link_t* connections, unsigned n);

size_t buf_size(unsigned pow_);

int
main(int argc, char* argv[])
{
    if(argc != 3){
        errno = E2BIG;
        error("bad number of arguments");
    }

    long n = strtol(argv[1], NULL, BASE);
    if((errno == ERANGE && (n == LONG_MAX || n == LONG_MIN)) ||
        (errno != 0 && n == 0) || n <= 0){
        fprintf(stderr, "number of children == %s\n", argv[1]);
        error("bad number of children");
    }

    int fd = open(argv[2], O_RDONLY);
    if(fd == -1)
        error("error in opening file with data");

    struct link_t * connections = calloc(n, sizeof(struct link_t));
    assert(connections);


    pid_t pid = 0;
    int i;
    for(i = 0; i < n; ++i){
        if(pipe(connections[i].p_in) == -1)
            error("error in pipe");
        if(i == n - 1){
            connections[i].p_out[1] = STDOUT_FILENO;
            connections[i].p_out[0] = EMPTY_FD;
        }else{
            if(pipe(connections[i].p_out) == -1)
                error("error in pipe");
        }

        pid = fork();

        /*Child close descriptors that it won't use at all*/
        if (pid == 0){
            if(i != 0)
                c_c(connections[i - 1].p_out[1]);
            c_c(connections[i].p_in[0]);
            break;
        }

        /*Parent close descriptors that it won't use and create children further*/
        else if(pid > 0){
            c_c(connections[i].p_in[1]);
            if(i != n - 1)
                c_c(connections[i].p_out[0]);

            connections[i].cur_max_fd = max(connections[i].p_in[0], connections[i].p_out[1]);
            if(i != 0 && connections[i].cur_max_fd < connections[i - 1].cur_max_fd)
                connections[i].cur_max_fd = connections[i - 1].cur_max_fd;
        }


        if(pid == -1)
            error("error in fork");
    }

    /*The continue of the child process*/
    if(!pid)
        i ? child(connections[i - 1].p_out[0], connections[i].p_in[1]) : child(fd, connections[i].p_in[1]);

    else{
        host(connections, n);
    }

    return 0;
}

void host(struct link_t* connections, unsigned n){
    assert(connections && n > 0);
    fd_set rd, wr;
    struct timeval timeout;
    timeout.tv_sec = SECONDS;
    timeout.tv_usec = M_SECONDS;

    //allocate memory for each parent buffer -- all in all [n] connections
    for(int i = 0; i < n; ++i){
        connections[i].size_ = buf_size(n - i);
        connections[i].buf = calloc(connections[i].size_, sizeof(char));
        assert(connections[i].buf);
    }

    int ip_ = 0;
    for(;;){
        FD_ZERO(&rd);
        FD_ZERO(&wr);


        for(int i = 0; i <= ip_; ++i){
            FD_SET(connections[ip_].p_in[0], &rd);
            FD_SET(connections[ip_].p_out[1], &wr);
        }

        int number_fd = select(connections[ip_].cur_max_fd, &rd, &wr, NULL, &timeout);
        if(number_fd == -1)
            error("error in select");
        if(number_fd == 0)
            break;

        int finished_operations = 0;
        while(finished_operations != number_fd){
            if(FD_ISSET(connections[ip_].p_in[0], &rd)){
                read_parent()
            }
        }
    }


    for(int j = 0; j < n; ++j)
        free(connections[j].buf);

    free(connections);
}

void child(int fd_read, int fd_write){
    assert(fd_read >= 0 && fd_write >= 0);
    char* buf = calloc(CHILD_BUF_SIZE, sizeof(char));
    assert(buf);

    ssize_t bytes_r = 0;
    ssize_t bytes_w = 0;
    while((bytes_r = read(fd_read, buf, CHILD_BUF_SIZE)) > 0){

        if((bytes_w = write(fd_write, buf, bytes_r)) == -1)
            error("error write");

        assert(bytes_r == bytes_w);

    }

    if(bytes_r == -1)
        error("error read");


    free(buf);
    c_c(fd_read);
    c_c(fd_write);
    exit(EXIT_SUCCESS);
}


size_t buf_size(unsigned pow_){

    errno = 0;
    double size = pow(3.0, (double)pow_);
    if(errno != 0)
        error("error in pow");

    if(size > (double)CHILD_BUF_SIZE)
        return CHILD_BUF_SIZE;

    return (size_t)size * 1024;

}
