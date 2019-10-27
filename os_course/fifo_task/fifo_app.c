//
// Created by denis on 25.09.19.
//

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <limits.h>
#include <features.h>
#include <sys/ioctl.h>
#include <assert.h>


enum Error_Code {
    ERROR_IN_READ_FILE = -1,
    ERROR_OPENING_WRITE_END = -2,
    ERROR_FCNTL = -3
};

#define SUCCESS 1
#define FIFO_MODE 0644
#define PIPE_BUF 4096
#define TIME_SLEEP 5

#define ERROR_MESSAGE(str)\
    printf("ERROR in line : %d\n", __LINE__);\
    printf("The errno value is : %d\n", errno);\
    perror(str);\
    exit(EXIT_FAILURE)\

#define SERVER_FIFO "pivotal_fifo"
#define CLIENT_FIFO_TEMPLATE "cfifo.%d"

//the maximum size_of_bytes in a name of private fifo
#define CLIENT_FIFO_NAME_LEN 17

char CLIENT_FIFO_PATH[CLIENT_FIFO_NAME_LEN];

void Writer_Processes(const char *path_to_data);

void Reader_Processes();

void Create_Name_Private_Fifo(const pid_t pid);

int Read_Pid(int pivotal_fifo_fd, pid_t *pid);

int Remove_O_NonBlock(int file_des);

int main(int argc, char *argv[]) {

    if (argc == 1) {
        Reader_Processes();

    } else if (argc == 2) {
        Writer_Processes(argv[1]);
    } else {
        printf("Too many arguments\n");
        exit(EXIT_FAILURE);
    }
}

void Writer_Processes(const char *path_to_data) {
    assert(path_to_data);

    int fd_pivotal_fifo = 0;
    pid_t pid = getpid();
    char buf[PIPE_BUF] = {};

    int data_file_fd = open(path_to_data, O_RDONLY);

    if (data_file_fd == -1) {
        ERROR_MESSAGE("error opening file with data");
    }

    //----------------------------------------------------------

    Create_Name_Private_Fifo(pid);

    if (mkfifo(CLIENT_FIFO_PATH, FIFO_MODE) == -1 && errno != EEXIST) {
        ERROR_MESSAGE("cannot create a client fifo");
    }

    int private_fifo_read_end = open(CLIENT_FIFO_PATH, O_RDONLY | O_NONBLOCK);
    int private_fifo_write_end = open(CLIENT_FIFO_PATH, O_WRONLY);

    if (private_fifo_write_end == -1 || private_fifo_read_end == -1) {
        ERROR_MESSAGE("error in opening private fifo");
    }

    if (mkfifo(SERVER_FIFO, FIFO_MODE) == -1 && errno != EEXIST) {
        ERROR_MESSAGE(SERVER_FIFO);
    }

    fd_pivotal_fifo = open(SERVER_FIFO, O_WRONLY);
    if (fd_pivotal_fifo == -1) {
        ERROR_MESSAGE("error in opening pivotal fifo for writing");
    }


    if (write(fd_pivotal_fifo, &pid, sizeof(pid_t)) != sizeof(pid_t)) {
        ERROR_MESSAGE("error in writing data in pivotal fifo");
    }

    int size_read = 0;
    int size_write = 0;

    sleep(TIME_SLEEP);
    close(private_fifo_read_end);
    //when the read process dies the write process will die too with SIGPIPE and the errno == EPIPE

    while ((size_read = read(data_file_fd, buf, PIPE_BUF)) != 0 && size_read != -1) {
        size_write = write(private_fifo_write_end, buf, size_read);
        if (size_write == -1) {
            ERROR_MESSAGE("error in writing data to private fifo");
        }
        /*this represents the case of a strange error when write haven't made an atomic operation*/
        if (size_read != size_write) {
            ERROR_MESSAGE("write end is blocked and the read end was died");
        }

    }
    if (size_read == -1) {
        ERROR_MESSAGE("error in reading file with data");
    }

    close(private_fifo_write_end);
    close(fd_pivotal_fifo);
    close(data_file_fd);
    exit(EXIT_SUCCESS);
}


void Reader_Processes() {
    int pivotal_fifo_fd = 0;
    pid_t pid = 0;

    char buf[PIPE_BUF] = {};

    if (mkfifo(SERVER_FIFO, FIFO_MODE) == -1 && errno != EEXIST) {
        ERROR_MESSAGE(SERVER_FIFO);
    }


    pivotal_fifo_fd = open(SERVER_FIFO, O_RDONLY);
    if (pivotal_fifo_fd == -1) {
        ERROR_MESSAGE("cannot open pivotal fifo for reading");
    }

    int read_r = Read_Pid(pivotal_fifo_fd, &pid);
    if (read_r != SUCCESS) {
        ERROR_MESSAGE("error in reading pid from pivotal fifo");
    }

    Create_Name_Private_Fifo(pid);

    int client_data = open(CLIENT_FIFO_PATH, O_RDONLY | O_NONBLOCK);
    if (client_data == -1) {
        ERROR_MESSAGE("file with data cannot be read");
    }

    if (Remove_O_NonBlock(client_data) < 0) {
        ERROR_MESSAGE("error in fcntl");
    }

    int size_read;
    while ((size_read = read(client_data, buf, PIPE_BUF)) != 0 && size_read != -1) {

        if (write(STDOUT_FILENO, buf, size_read) == -1) {
            ERROR_MESSAGE("error writing to stdout");
        }

    }
    if (size_read == -1) {
        ERROR_MESSAGE("error reading client file");
    }


    close(pivotal_fifo_fd);
    close(client_data);
    unlink(CLIENT_FIFO_PATH);
    exit(EXIT_SUCCESS);
}

int Remove_O_NonBlock(int file_des) {
    int flag = fcntl(file_des, F_GETFL);
    if (flag == -1)
        return ERROR_FCNTL;
    flag &= ~O_NONBLOCK;
    if (fcntl(file_des, F_SETFL, flag) == -1)
        return ERROR_FCNTL;
    return SUCCESS;
}

int Read_Pid(int pivotal_fifo_fd, pid_t *pid) {

    assert(pid);

    int read_f = read(pivotal_fifo_fd, pid, sizeof(pid_t));
    // the case when all the write end is closed and the fifo_buf is empty (EOF returned)
    if (read_f == 0) {

        //it will succeed immediatelly due to this process has already opened read_end
        int write_end = open(SERVER_FIFO, O_WRONLY);
        if (write_end == -1) {
            return ERROR_OPENING_WRITE_END;
        }

        // here the process must wait the writer
        int wait_read = read(pivotal_fifo_fd, pid, sizeof(pid_t));

        //also the cases when errno is EAGAIN
        if (wait_read != sizeof(pid_t))
            return ERROR_IN_READ_FILE;

        close(write_end);


        return SUCCESS;
    } else if (read_f != sizeof(pid_t))
        return ERROR_IN_READ_FILE;

    return SUCCESS;
}

void Create_Name_Private_Fifo(const pid_t pid) {
    memset(CLIENT_FIFO_PATH, '\0', CLIENT_FIFO_NAME_LEN);
    snprintf(CLIENT_FIFO_PATH, CLIENT_FIFO_NAME_LEN, CLIENT_FIFO_TEMPLATE, pid);
}