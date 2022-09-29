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

typedef struct {
    struct PCBTable pcb[200];
    int in;
} PCB_wrapper;
int shmid;
PCB_wrapper* shm;

void add_process(PCB_wrapper* pcb_wrapper, pid_t p, int stat) {
    int i = (*pcb_wrapper).in;

    ((*pcb_wrapper).pcb[i]).pid = p;
    ((*pcb_wrapper).pcb[i]).status = stat;
    ((*pcb_wrapper).pcb[i]).exitCode = -1;

    (*pcb_wrapper).in++;
}

void exit_process(PCB_wrapper* pcb_wrapper, pid_t p, int exitcode) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        if (((*pcb_wrapper).pcb[i]).pid == p) {
            ((*pcb_wrapper).pcb[i]).status = 1; //exited
            ((*pcb_wrapper).pcb[i]).exitCode = exitcode;
        }
    }
}

void block_process(PCB_wrapper* pcb_wrapper, pid_t p) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        if (((*pcb_wrapper).pcb[i]).pid == p) {
            ((*pcb_wrapper).pcb[i]).status = 4; //stopped
        }
    }
}

void unblock_process(PCB_wrapper* pcb_wrapper, pid_t p) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        if (((*pcb_wrapper).pcb[i]).pid == p) {
            ((*pcb_wrapper).pcb[i]).status = 2; //running
        }
    }
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
    shm[1].in = 0; //Use shm[1].in as a flag for whether the PCB is free(0) or being modified(1);
    return;
}

void my_process_command(size_t num_tokens, char **tokens) {
    int tokenNo = (int) num_tokens;
    char* command = tokens[0];

    if (!strcmp(command, "info")) {
        while(shm[1].in == 1) {
            if (shm[1].in == 0) {
                shm[1].in = 1;
                if (!strcmp(tokens[1], "0")) {
                    for (int i = 0; i < shm[0].in; i++) {
                        if (shm[0].pcb[i].exitCode != -1) {
                            printf("[%d] Exited %d\n", shm[0].pcb[i].pid, shm[0].pcb[i].exitCode);
                        } else {
                            printf("[%d] Running\n", shm[0].pcb[i].pid);
                        }
                    }
                } else if(!strcmp(tokens[1], "1")) {
                    int exitCount = 0;
                    for (int i = 0; i < shm[0].in; i++) {
                        if (shm[0].pcb[i].exitCode != 1) {
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
                } else {
                    fprintf(stderr, "Wrong command\n");
                }
                shm[1].in = 0;
                return;
            }
        }
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
        PCB_wrapper* shm1 = shmat( shmid, NULL, 0);
        while(0 == 0) {
            if (shm1[1].in == 0) {
                shm1[1].in == 1;
                add_process(&shm1[0], childpid, 2);
                shm1[1].in == 0;
                break;
            }
        }
        execvp(command, tokens);
        //returns if exec throws error
        fprintf(stderr, "%s not found\n", command);
        while(0 == 0) {
            if (shm1[1].in == 0) {
                shm1[1].in == 1;
                exit_process(&shm1[0], childpid, 1);
                shm1[1].in == 0;
                break;
            }
        }
        exit(1);   
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
    pid_t curr_pid = getpid();
    for(int i = 0; i < shm[0].in; i++) { 
        shm[0].pcb[i].status = 3; //terminated
        shm[0].pcb[i].exitCode = 143; //SIGTERM
        if ((shm[0].pcb[i].pid != curr_pid) && ((shm[0].pcb[i].status == 4) || (shm[0].pcb[i].status == 2))) {
            printf("Killing [%d]", shm[0].pcb[i].pid);
            kill(shm[0].pcb[i].pid, SIGTERM);
        }
    }
    printf("Killing [%d]\n", curr_pid);
    printf("\nGoodbye\n");
    kill(curr_pid, SIGTERM);
}
