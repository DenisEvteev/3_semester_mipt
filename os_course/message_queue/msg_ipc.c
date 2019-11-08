//
// Created by denis on 08.10.19.
//

#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#include <sys/types.h>
#include <unistd.h>
#include <assert.h>

#define ERROR_MESSAGE(str)\
    printf("error in line : %d\n", __LINE__);\
    printf("error in function : %s\n", __PRETTY_FUNCTION__);\
    printf("errno is equal to : %d\n", errno);\
    perror(str);\
    exit(EXIT_FAILURE)\


enum {
    FIRST_MESSAGE =  1,
    QUEUE_MODE =  0666,
    BASE = 10

};

struct msgbuf {
    long mtype;
};

enum Error_Code{
    ERROR_STRTOL = -1
};


int Get_Number_Children(const char* value);
void Parent_Processing(int msg_id, long children_number);
void Child_Processing(int msg_id, long i);

int main(int argc, char* argv[]){
    int msg_id = 0;
    const pid_t pid_parent = getpid();


    setbuf(stdout, NULL);
    if(argc != 2){
       ERROR_MESSAGE("bad number of arguments");
    }

    long ch_count = Get_Number_Children(argv[1]);

    if(ch_count > 0){
        msg_id = msgget(IPC_PRIVATE, QUEUE_MODE);
        if(msg_id == -1){
            ERROR_MESSAGE("error in msgget");
        }
    }
    else{
        ERROR_MESSAGE("bad number of children");
    }

    errno = 0;

    pid_t pid;
    long i;
    for(i = 0; i < ch_count; ++i){
        pid = fork();

        if(pid == 0)
            break;

        else if(pid < 0){
            if(msgctl(msg_id, IPC_RMID, NULL) == -1)
                printf("error in msgctl\n");
            ERROR_MESSAGE("error fork");
        }
    }

    if(getpid() == pid_parent)
        Parent_Processing(msg_id, ch_count);
    else
        Child_Processing(msg_id, i);


    exit(EXIT_SUCCESS);
}

void Parent_Processing(int msg_id, long children_number){
    assert(msg_id >= 0 && children_number > 0);
    /*actually zero messages require one byte for saving their empty text field in the message queue,
     * so their limit in my system is MSGMNB == 16384, if this number is exceeded  then msgsnd is blocked*/
    struct msgbuf msg = {FIRST_MESSAGE};
    if(msgsnd(msg_id, &msg, 0, 0) == -1){
        ERROR_MESSAGE("error in msgsnd");
    }

    //wait the last child --- blocking call
    if(msgrcv(msg_id, &msg, 0, children_number + 1, 0) == -1){
        ERROR_MESSAGE("error in the last sinchranization");
    }

    if(msgctl(msg_id, IPC_RMID, NULL) == -1){
        ERROR_MESSAGE("error in msgctl");
    }

    exit(EXIT_SUCCESS);

}

void Child_Processing(int msg_id, long i) {
    assert(msg_id >= 0);

    struct msgbuf msg;
    if (msgrcv(msg_id, &msg, 0, ++i, 0) == -1){
        ERROR_MESSAGE("error in the first msgrcv");
    }

    printf("%ld ", i++);

    struct msgbuf msg_send = {i};

    if (msgsnd(msg_id, &msg_send, 0, 0) == -1) {
        ERROR_MESSAGE("error in msgsnd");
    }

}

int Get_Number_Children(const char* value){
    assert(value);

    errno = 0;

    long number_children = strtol(value, NULL, BASE);

    if((errno == ERANGE && (number_children == LONG_MAX || number_children == LONG_MIN)) ||
    (errno != 0 && number_children == 0)){
        return ERROR_STRTOL;
    }

    return number_children;
}