//
//   created by denis on the 07.02.2019
//

/*Regarding zombies, UNIX systems imitate the movies — a zombie process can’t be
killed by a signal, not even the (silver bullet) SIGKILL

 C’est la vie   ;)


On the other
hand, if the parent terminates without doing a wait(), then the init process adopts
the child and automatically performs a wait(), thus removing the zombie process
from the system.


 default action on deliverying SIGCHLD signal is ignoring*/

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <assert.h>
#include <setjmp.h>
#include <wait.h>


/*Signal disposition and signal mask is inherited via fork()*/
#define NUMBER_BITS 7

#define NORETURN __attribute__((noreturn))

int GLOBAL_BIT;
sigjmp_buf env;

#define sys_error(str)                                       \
do{                                                           \
    fprintf(stderr, "LINE : %d\n", __LINE__);                  \
    fprintf(stderr, "FUNCTION : %s\n", __PRETTY_FUNCTION__);    \
    fprintf(stderr, "ERRNO : %d\n", errno);                      \
    perror(str);                                                  \
    exit(EXIT_FAILURE);                                            \
}while(0)                                                           \



//Let's SIGUSR1 === 0
//      SIGUSR2 === 1


void ipc_handler(int sig);

void par_exit(int sig);

void handler_sigio(int sig);

void handler_sigio_parent(int sig);

void child(const char *file_path, int parent_pid) NORETURN;

void parent(pid_t child_pid)NORETURN;

void setting_mask_handlers();

/*I have such a strange problem (aka bug) in my program that I must fix it
 * with the same input data the program give different output result that is a very strange fact*/

int main(int argc, char **argv) {
    if (argc != 2) {
        errno = E2BIG;
        sys_error("argv");
    }
    pid_t parent_pid = getpid();

    setting_mask_handlers();


    pid_t pid = fork();

    if (pid > 0) // parent
        parent(pid);
    else if (pid == 0)
        child(argv[1], parent_pid);
    else
        sys_error("fork");


    return 0;
}


void setting_mask_handlers() {
    sigset_t block_mask;
    if (sigaddset(&block_mask, SIGUSR1))
        sys_error("sigaddset SIGUSR1 in block_mask");
    if (sigaddset(&block_mask, SIGUSR2))
        sys_error("sigaddset SIGUSR2 in block_mask");
    if (sigaddset(&block_mask, SIGIO))
        sys_error("sigaddset SIGIO in block_mask");
    if (sigprocmask(SIG_SETMASK, &block_mask, NULL))
        sys_error("sigprocmask");

    struct sigaction act_1;
    act_1.sa_handler = ipc_handler;
    sigaddset(&act_1.sa_mask, SIGUSR2);
    act_1.sa_flags = 0;
    if (sigaction(SIGUSR1, &act_1, NULL) == -1)
        sys_error("sigaction SIGUSR1");

    struct sigaction act_2;
    act_2.sa_handler = ipc_handler;
    sigaddset(&act_2.sa_mask, SIGUSR1);
    act_2.sa_flags = 0;
    if (sigaction(SIGUSR2, &act_2, NULL) == -1)
        sys_error("sigaciton SIGUSR2");

    struct sigaction act_3;
    act_3.sa_handler = par_exit;
    act_3.sa_mask = block_mask; // Don't allow this signals to mix the normal jump in handler of SIGCHLD
    act_3.sa_flags = 0;
    if (sigaction(SIGCHLD, &act_3, NULL) == -1)
        sys_error("sigaction SIGCHLD");

    struct sigaction act_4;
    act_4.sa_handler = handler_sigio_parent;
    act_4.sa_flags = 0;
    if (sigfillset(&act_4.sa_mask) == -1)
        sys_error("sigfillset");
    if (sigaction(SIGIO, &act_4, NULL) == -1)
        sys_error("sigaction SIGIO");
}

void child(const char *file_path, int parent_pid) {
    assert(file_path && parent_pid >= 0);
    if (prctl(PR_SET_PDEATHSIG, SIGKILL) == -1)
        sys_error("prctl PR_SET_PDEATHSIG");

    if (getppid() != parent_pid)
        exit(EXIT_FAILURE);

    struct sigaction act;
    act.sa_handler = handler_sigio;
    act.sa_flags = 0;
    if (sigemptyset(&act.sa_mask))
        sys_error("sigemptyset handler_sigio");
    if (sigaction(SIGIO, &act, NULL) == -1)
        sys_error("sigaction SIGIO in the child");

    int fd = open(file_path, O_RDONLY);
    if (fd < 0)
        sys_error("open");

    char single_read = '\0';
    ssize_t bytes = 0;

    sigset_t clear_mask;
    if (sigemptyset(&clear_mask))
        sys_error("sigemptyset clear_mask");


    while ((bytes = read(fd, &single_read, sizeof(char))) > 0) {
        for (int i = NUMBER_BITS; i >= 0; --i) {

            if ((1 << i) & single_read) {
                if (kill(parent_pid, SIGUSR2) == -1)
                    sys_error("kill SIGUSR2");
            } else {
                if (kill(parent_pid, SIGUSR1) == -1)
                    sys_error("kill SIGUSR1");
            }

            if (sigsuspend(&clear_mask) == -1 && errno != EINTR) //block here with undefined reason
                sys_error("sigsuspend in child");
        }
    }
    if (bytes < 0)
        sys_error("read in child");

    if (kill(parent_pid, SIGIO) == -1)
        sys_error("kill SIGIO");

    close(fd);
    exit(EXIT_SUCCESS);
}

void parent(pid_t child_pid) {
    assert(child_pid >= 0);
    char letter = '\0';
    sigset_t zero;
    sigemptyset(&zero);

    if (sigsetjmp(env, 0) != 0) {
        wait(NULL);
        _exit(EXIT_SUCCESS);
    }

    for (;;) { //endless loop in parent --- the reason for going out from it is SIGCHLD signal

        for (int i = NUMBER_BITS; i >= 0; --i) {
            if (sigsuspend(&zero) == -1 && errno != EINTR)
                sys_error("sigsuspend in parent");
            if (GLOBAL_BIT == SIGUSR2) // bit 1 -- set 1 to the i position
                letter |= (1 << i);
            else if (GLOBAL_BIT == SIGUSR1) // bit 0 -- set 0 to the i position
                letter &= ~(1 << i);
            /*Send the notification to the child that it can
             * transfer the data further*/
            if (i == 0) {
                int write_b = write(STDOUT_FILENO, &letter, sizeof(char));
                if (write_b != sizeof(char))
                    sys_error("write to stdout");
            }
            if (kill(child_pid, SIGIO) == -1)
                sys_error("kill in parent");
        }
    }
}


void ipc_handler(int sig) {
    GLOBAL_BIT = sig;
}

void par_exit(int sig) {
    _exit(EXIT_FAILURE);
}

void handler_sigio(int sig) {}

void handler_sigio_parent(int sig) {
    struct sigaction act;
    act.sa_handler = SIG_DFL;
    if (sigaction(SIGIO, &act, NULL) == -1)
        sys_error("sigaction SIGIO");

    siglongjmp(env, sig);
}
