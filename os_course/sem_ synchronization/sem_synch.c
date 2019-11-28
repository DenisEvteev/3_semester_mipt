//
// Created by denis on 12.11.19.
//
#include <stdio.h>
#include <limits.h>
#include <sys/ipc.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <unistd.h>

#define NORETURN __attribute__((noreturn))

#define PROJECT_ID_SEM '$'
#define PROJECT_ID_SHM '*'

enum
{
    NUMBER_SEMAPHORES = 4,
    PERMS = 0666,
    DECREASE = -1,
    INCREASE = 1,
    SUBTRACT_TWO = -2,
    ADD_TWO = 2,
    SIZE_BUF = 4092,
    MAX_NUMBER_OF_OPERATIONS = 7

};

enum SEMAPHORE_TYPE
{
    SYNCH_WRITERS,
    SYNCH_READERS,
    ENTRY_WRITING,
    ENTRY_READING
};

/*I assign
the return value from shmat() to a pointer to some programmer-defined structure,
in order to impose that structure on the segment*/
struct shmseg
{
    unsigned int bytes;
    char buf[SIZE_BUF];
};

/*global static array for performing different kind of operations on
 * the semaphore set.
 * Function mark_operation_sem will fill some elements of this array
 * Then function apply_operations_sem will perform atomically the whole set of operations on the semaphore set
 * enable fuction semop for semaphore set
 *
 * Due to the maximum number of operation to perform on the semaphore set is 5 so the size of this
 * array is corresponding*/
struct sembuf sops[MAX_NUMBER_OF_OPERATIONS] = {};

/*This global variable will be the index in the array of semaphore operations*/
short num_oper;

#define BAD_ADR (struct shmseg*)-1

/*Print just an message about the error and corresponding value of errno*/
void
err_exit(const char *str) NORETURN;
int
create_semaphore_set();
int
create_shared_memory_object();
void
reader(int sem_id, int shm_id) NORETURN;
void
writer(const char *file_path, int sem_id, int shm_id) NORETURN;
void
mark_operation_sem(int sem_id, int semnum, short flag, short type_operation);
void
apply_operations_sem(int sem_id, short number);

int
main(int argc, char *argv[])
{

    errno = 0;

    int sem_id = create_semaphore_set();

    int shm_id = create_shared_memory_object();


    switch (argc)
    {
        case 1 :
        {
            reader(sem_id, shm_id);
        }

        case 2 :
        {
            writer(argv[1], sem_id, shm_id);
        }

        default :err_exit("bad number of arguments");
    }
}

void
writer(const char *file_path, int sem_id, int shm_id)
{
    assert(file_path && sem_id >= 0 && shm_id >= 0);
    short id = 0;
    short flag = 0;

    struct shmseg *segment = shmat(shm_id, NULL, 0);
    if (segment == BAD_ADR)
        err_exit("error in shmat in writer process");

    int file_data_fd = open(file_path, O_RDONLY);
    if (file_data_fd == -1)
        err_exit("error in opening data file");

    mark_operation_sem(sem_id, SYNCH_WRITERS, 0, 0);
    mark_operation_sem(sem_id, SYNCH_WRITERS, SEM_UNDO, INCREASE);
    mark_operation_sem(sem_id, ENTRY_WRITING, 0, DECREASE);
    mark_operation_sem(sem_id, ENTRY_WRITING, 0, INCREASE);
    mark_operation_sem(sem_id, SYNCH_READERS, 0, DECREASE);
    mark_operation_sem(sem_id, SYNCH_READERS, SEM_UNDO, INCREASE);
    mark_operation_sem(sem_id, SYNCH_WRITERS, 0, INCREASE);
    apply_operations_sem(sem_id, 7);

    for (;;)
    {

        mark_operation_sem(sem_id, SYNCH_READERS, IPC_NOWAIT, DECREASE);
        mark_operation_sem(sem_id, SYNCH_READERS, 0, INCREASE);
        mark_operation_sem(sem_id, ENTRY_WRITING, 0, DECREASE);
        apply_operations_sem(sem_id, 3);

        segment->bytes = read(file_data_fd, segment->buf, SIZE_BUF);

        if (segment->bytes == -1)
            err_exit("error in reading data from file");

        if (id == 0)
        {
            flag = SEM_UNDO;
            id = 1;
        }
        else
            flag = 0;

        mark_operation_sem(sem_id, SYNCH_READERS, IPC_NOWAIT, DECREASE);
        mark_operation_sem(sem_id, SYNCH_READERS, 0, INCREASE);
        mark_operation_sem(sem_id, ENTRY_READING, flag, INCREASE);
        apply_operations_sem(sem_id, 3);

        /*When file with data is entirely transferred and the current offset at the end of it
         * then read returns zero so we break from the loop*/
        if (segment->bytes == 0)
            break;
    }

    if (shmdt(segment) == -1)
        err_exit("error in shmdt in writer process");

    close(file_data_fd);


    /*process blocks until reader change the value of the semaphore
     * for writing; only after the read process detach the shared memory
     * the write process check of the existence of other processes and remove the sources*/
    mark_operation_sem(sem_id, SYNCH_READERS, IPC_NOWAIT, DECREASE);
    mark_operation_sem(sem_id, SYNCH_READERS, 0, INCREASE);
    mark_operation_sem(sem_id, ENTRY_WRITING, 0, DECREASE);
    mark_operation_sem(sem_id, SYNCH_READERS, SEM_UNDO, DECREASE);
    mark_operation_sem(sem_id, SYNCH_WRITERS, SEM_UNDO, DECREASE);
    apply_operations_sem(sem_id, 5);


    exit(EXIT_SUCCESS);
}

void
reader(int sem_id, int shm_id)
{
    assert(sem_id >= 0 && shm_id >= 0);
    u_int recorded_bytes = 0;

    struct shmseg *segment = shmat(shm_id, NULL, SHM_RDONLY);
    if (segment == BAD_ADR)
        err_exit("error in shmat in reader process");

    mark_operation_sem(sem_id, SYNCH_READERS, 0, 0);
    mark_operation_sem(sem_id, SYNCH_WRITERS, 0, 0);
    mark_operation_sem(sem_id, SYNCH_READERS, SEM_UNDO, INCREASE);
    mark_operation_sem(sem_id, ENTRY_WRITING, SEM_UNDO, INCREASE);
    mark_operation_sem(sem_id, SYNCH_WRITERS, SEM_UNDO, INCREASE);
    mark_operation_sem(sem_id, SYNCH_WRITERS, 0, DECREASE);
    apply_operations_sem(sem_id, 6);

    mark_operation_sem(sem_id, SYNCH_WRITERS, 0, DECREASE);
    mark_operation_sem(sem_id, SYNCH_WRITERS, 0, INCREASE);
    apply_operations_sem(sem_id, 2);


    for (;;)
    {

        mark_operation_sem(sem_id, SYNCH_WRITERS, IPC_NOWAIT, SUBTRACT_TWO);
        mark_operation_sem(sem_id, SYNCH_WRITERS, 0, ADD_TWO);
        mark_operation_sem(sem_id, ENTRY_READING, 0, DECREASE);
        apply_operations_sem(sem_id, 3);

        /*As we have attached the shared memory to the virtual space of read process, so
         * as soon as the write process write some data in it. It entirely become avaliable to the
         * read process, so that we can write the data to the stdout stream
         *
         * BUT NOTICE : The first four bytes of it will be busy with the size of the recorded data
         * so I'm going to write just the information starting with a bit different pointer*/
        recorded_bytes = write(STDOUT_FILENO, segment->buf, segment->bytes);
        if (recorded_bytes == -1)
            err_exit("error in writing data from shared memory to stdout");

        //check if we read all right number of bytes from shared memory
        assert(recorded_bytes == segment->bytes);

        mark_operation_sem(sem_id, SYNCH_WRITERS, IPC_NOWAIT, SUBTRACT_TWO);
        mark_operation_sem(sem_id, SYNCH_WRITERS, 0, ADD_TWO);
        mark_operation_sem(sem_id, ENTRY_WRITING, 0, INCREASE);
        apply_operations_sem(sem_id, 3);

        if (segment->bytes == 0)
            break;
    }

    if (shmdt(segment) == -1)
        err_exit("error in shmdt in reader process");

    mark_operation_sem(sem_id, SYNCH_READERS, 0, 0);
    mark_operation_sem(sem_id, SYNCH_WRITERS, SEM_UNDO, DECREASE);
    mark_operation_sem(sem_id, ENTRY_WRITING, IPC_NOWAIT, 0);
    apply_operations_sem(sem_id, 3);

    if (shmctl(shm_id, IPC_RMID, NULL) == -1)
        err_exit("error in shmctl IPC_RMID");

    exit(EXIT_SUCCESS);
}

int
create_semaphore_set()
{
    int sem_id = 0;
    key_t key = ftok(__FILE__, PROJECT_ID_SEM);
    if (key == -1)
        err_exit("error in ftok for sem");

    sem_id = semget(key, NUMBER_SEMAPHORES, PERMS | IPC_CREAT);
    if (sem_id == -1)
        err_exit("error in semget");

    return sem_id;
}


int
create_shared_memory_object()
{
    int shm_id = 0;

    key_t key = ftok(__FILE__, PROJECT_ID_SHM);
    if (key == -1)
        err_exit("error in ftok for shm");

    shm_id = shmget(key, sizeof(struct shmseg), PERMS | IPC_CREAT);
    if (shm_id == -1)
        err_exit("error in shmget");

    return shm_id;
}

void
err_exit(const char *str)
{
    if (errno == 0)
        fprintf(stderr, str);

    else
    {
        fprintf(stderr, "%d\n", errno);
        perror(str);
    }

    exit(EXIT_FAILURE);
}

void
mark_operation_sem(int sem_id, int semnum, short flag, short type_operation)
{
    assert(sem_id >= 0 && semnum >= 0);

    sops[num_oper].sem_num = semnum;
    sops[num_oper].sem_op = type_operation;
    sops[num_oper].sem_flg = flag;
    ++num_oper;
}

void
apply_operations_sem(int sem_id, short number)
{
    assert(sem_id >= 0 && number > 0);

    if (semop(sem_id, sops, number) == -1)
        err_exit("error in semop");
    num_oper = 0;
}

#undef NORETURN