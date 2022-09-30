/**
 * CS2106 AY22/23 Semester 1 - Lab 2
 *
 * This file contains function definitions. Your implementation should go in
 * this file.
 */

#include "myshell.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/wait.h>

typedef struct {
    struct PCBTable pcb[200];
    int in;
    int flag;
} PCB_wrapper;
int shmid;
PCB_wrapper* shm;

void add_process(PCB_wrapper* pcb_wrapper, pid_t p, int stat) {
    int i = (*pcb_wrapper).in;
    while(1) {
        if ((*pcb_wrapper).flag == 0) {
            (*pcb_wrapper).flag = 1;
            ((*pcb_wrapper).pcb[i]).pid = p;
            ((*pcb_wrapper).pcb[i]).status = stat;
            ((*pcb_wrapper).pcb[i]).exitCode = -1;
            (*pcb_wrapper).flag = 0;
            break;
        }
    }
    (*pcb_wrapper).in++;
    return;
}

void exit_process(PCB_wrapper* pcb_wrapper, pid_t p, int exitcode) {
    while(1) {
        if ((*pcb_wrapper).flag == 0) {
            (*pcb_wrapper).flag = 1;
            for(int i = 0; i < (*pcb_wrapper).in; i++) {
                if (((*pcb_wrapper).pcb[i]).pid == p) {
                    ((*pcb_wrapper).pcb[i]).status = 1; //exited
                    ((*pcb_wrapper).pcb[i]).exitCode = exitcode;
                }
            }
            (*pcb_wrapper).flag = 0;
            break;
        }
    }
    return;
}

void terminating_process(PCB_wrapper* pcb_wrapper, pid_t p) {
    while(1) {
        if ((*pcb_wrapper).flag == 0) {
            (*pcb_wrapper).flag = 1;
            for(int i = 0; i < (*pcb_wrapper).in; i++) {
                if (((*pcb_wrapper).pcb[i]).pid == p) {
                    ((*pcb_wrapper).pcb[i]).status = 3; //terminating
                }
            }
            (*pcb_wrapper).flag = 0;
            break;
        }
    }
    return;
}

int exited(PCB_wrapper* pcb_wrapper, pid_t p) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        if (((*pcb_wrapper).pcb[i].pid == p) && ((*pcb_wrapper).pcb[i].status == 1)) {
            return 1;
        }
    }
    return 0;
}

int running(PCB_wrapper* pcb_wrapper, pid_t p) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        if (((*pcb_wrapper).pcb[i].pid == p) && ((*pcb_wrapper).pcb[i].status == 2)) {
            return 1;
        }
    }
    return 0;
}

void print_PCB(PCB_wrapper* pcb_wrapper) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        printf("pid: %d\n", ((*pcb_wrapper).pcb[i]).pid);
        printf("status: %d\n", ((*pcb_wrapper).pcb[i]).status);
        printf("exitCode: %d\n\n", ((*pcb_wrapper).pcb[i]).exitCode);
    }
}

void my_init(void) {
    // Create shared memory region
    shmid = shmget(IPC_PRIVATE, sizeof(PCB_wrapper), IPC_CREAT | 0666); //shmid -> shared memory id
    shm = shmat(shmid, NULL, 0); //attach main process to shared memory
    //Initialise PCB_wrapper in shm with dummy values
    shm[0].in = 0;
    for(int i = 0; i < 200; i++) {
        shm[0].pcb[i].pid = -1;
        shm[0].pcb[i].status = -1;
        shm[0].pcb[i].exitCode = -1;
    }
    shm[0].flag = 0; //Use shm[0].flag as a flag for whether the PCB is free(0) or being modified(1);
    // printf("shm[0].flag\n");
    return;
}

void my_process_command(size_t num_tokens, char **tokens) {
    int tokenNo = (int) num_tokens;
    char* command = tokens[0];

    if (!strcmp(command, "wait")) {
        if (tokens[1] == NULL) {
            fprintf(stderr, "Wrong command\n");
        } else {
            pid_t pid_arg = strtol(tokens[1], NULL, 0);
            if (running(&shm[0], pid_arg)) {
                int status;
                waitpid(pid_arg, &status, 0);
            }
        }
        return;
    }

    if (!strcmp(command, "terminate")) {
        if (tokens[1] == NULL) {
            fprintf(stderr, "Wrong command\n");
        } else {
            pid_t pid_arg = strtol(tokens[1], NULL, 0);
            if (running(&shm[0], pid_arg)) {
                terminating_process(&shm[0], pid_arg);
                kill(pid_arg, SIGTERM);
            }
        }
        return;
    }

    if (!strcmp(command, "info")) {
        // print_PCB(&shm[0]);
        if (tokens[1] == NULL) {
            fprintf(stderr, "Wrong command\n");
        } else {
            if (!strcmp(tokens[1], "0")) {
                // print_PCB(&shm[0]);
                for (int i = 0; i < shm[0].in; i++) {
                    if (shm[0].pcb[i].status == 1) {
                        printf("[%d] Exited %d\n", shm[0].pcb[i].pid, shm[0].pcb[i].exitCode);
                    } else if (shm[0].pcb[i].status == 2){
                        printf("[%d] Running\n", shm[0].pcb[i].pid);
                    } else if(shm[0].pcb[i].status == 3) {
                        printf("[%d] Terminating\n", shm[0].pcb[i].pid);
                    }
                }
            } else if(!strcmp(tokens[1], "1")) {
                int exitCount = 0;
                for (int i = 0; i < shm[0].in; i++) {
                    if (shm[0].pcb[i].status == 1) {
                        exitCount++;
                    } 
                }
                printf("Total exited process: %d\n", exitCount);
            } else if(!strcmp(tokens[1], "2")) {
                int runningCount = 0;
                for (int i = 0; i < shm[0].in; i++) {
                    if (shm[0].pcb[i].status == 2) {
                        runningCount++;
                    } 
                }
                printf("Total running process: %d\n", runningCount);
            } else if(!strcmp(tokens[1], "3")) {
                int terminatingCount = 0;
                for (int i = 0; i < shm[0].in; i++) {
                    if (shm[0].pcb[i].status == 3) {
                        terminatingCount++;
                    } 
                }
                printf("Total running process: %d\n", terminatingCount);
            } else {
                fprintf(stderr, "Wrong command\n");
            }
        }        
        return;
    } 

    // If last string is "&" remove it from the arguments
    int w = 1;
    if (!strcmp(tokens[tokenNo - 2], "&")) {
        tokens[tokenNo - 2] = NULL;
        w = 0;
    }

    // Fork child process
    pid_t childpid = fork();

    if (childpid == 0) {
        // Attach to shared memory in child process
        PCB_wrapper* shm1 = shmat(shmid, NULL, 0);
        add_process(&shm1[0], getpid(), 2);
        
        
        pid_t curr_pid = getpid();
        pid_t grandchildpid = fork();
        //Run exec in grandchild process so that exit can be logged
        if (grandchildpid == 0) {
            execvp(command, tokens);
            // fprintf(stderr, "%s not found\n", command);
            // // PCB_wrapper* shm2 = shmat(shmid, NULL, 0);
            // // exit_process(&shm2[0], curr_pid, 127);
            exit(127); //command not found
        }
        else {
            int status;
            if (waitpid(grandchildpid, &status, 0) == -1) {
                perror("waitpid failed");
                exit(1);
            }

            if (WIFEXITED(status)) {
                const int exit_code = WEXITSTATUS(status);
                // printf("exit code: %d\n", exit_code);
                if (exit_code == 127) {
                    fprintf(stderr, "%s not found\n", command);
                } 
                exit_process(&shm1[0], curr_pid, exit_code);
                exit(0);
            }
        }   
    } 
    
    // Block current process and wait for child process to exit if wait == 1
    // Otherwise print "Child [%d] in background" and exit
    if(w == 1) {
        wait(NULL);
    } else {
        printf("Child [%d] in background\n", childpid);
    }
    return;
}

void my_quit(void) {
    // pid_t curr_pid = getpid();
    for(int i = 0; i < shm[0].in; i++) { 
        if (shm[0].pcb[i].status == 2) {
            shm[0].pcb[i].status = 3; //terminated
            shm[0].pcb[i].exitCode = 143; //SIGTERM
            printf("Killing [%d]\n", shm[0].pcb[i].pid);
            kill(shm[0].pcb[i].pid, SIGTERM);
        }
    }
    printf("\nGoodbye\n");
}
