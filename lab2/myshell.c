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
int shmid, *shm;

void add_process(PCB_wrapper pcb_wrapper, pid_t p, int stat) {
    int i = pcb_wrapper.in;
    struct PCBTable next_table = pcb_wrapper.pcb[i];

    next_table.pid = p;
    next_table.status = stat;
    next_table.exitCode = -1;

    pcb_wrapper.in++;
}

void exit_process(PCB_wrapper pcb_wrapper, pid_t p, int exitcode) {

    for(int i = 0; i < pcb_wrapper.in; i++) {
        if (pcb_wrapper.pcb[i].pid == p) {
            pcb_wrapper.pcb[i].status = 1; //exited
            pcb_wrapper.pcb[i].exitCode = exitcode;
        }
    }
}

void block_process(PCB_wrapper pcb_wrapper, pid_t p) {
    for(int i = 0; i < pcb_wrapper.in; i++) {
        if (pcb_wrapper.pcb[i].pid == p) {
            pcb_wrapper.pcb[i].status = 4; //stopped
        }
    }
}

void unblock_process(PCB_wrapper pcb_wrapper, pid_t p) {
    for(int i = 0; i < pcb_wrapper.in; i++) {
        if (pcb_wrapper.pcb[i].pid == p) {
            pcb_wrapper.pcb[i].status = 2; //running
        }
    }
} 

void print_PCB(PCB_wrapper pcb_wrapper) {
    for(int i = 0; i < pcb_wrapper.in; i++) {
        printf("pid: %d\n", pcb_wrapper.pcb[i].pid);
        printf("status: %d\n", pcb_wrapper.pcb[i].status);
        printf("exitCode: %d\n\n", pcb_wrapper.pcb[i].exitCode);
    }
}

void my_init(void) {
    PCB_wrapper* pcb_state;
    
    // Create shared memory region
    shmid = shmget(IPC_PRIVATE, 40, IPC_CREAT | 0600); //shmid -> shared memory id
    shm = (int*) shmat(shmid, NULL, 0); //attach main process to shared memory
    shm[1] = pcb_state;
    // shm[0] = 0 for free; shm[0] = 1 for writing
    shm[0] = 0;
}

void my_process_command(size_t num_tokens, char **tokens) {
    int tokenNo = (int) num_tokens;
    char* command = tokens[0];
    PCB_wrapper info_pcb = * (PCB_wrapper*) shm[1];

    if (!strcmp(command, "info")) {
        if (!strcmp(tokens[1], "0")) {
            for (int i = 0; i < info_pcb.in; i++) {
                if (info_pcb.pcb[i].status == 1 || info_pcb.pcb[i].status == 3) {
                    printf("[%d] Exited %d\n", info_pcb.pcb[i].pid, info_pcb.pcb[i].exitCode);
                } else {
                    printf("[%d] Running\n", info_pcb.pcb[i].pid);
                }
            }

        } else if(!strcmp(tokens[1], "1")) {
            int exitCount = 0;
            for (int i = 0; i < info_pcb.in; i++) {
                if (info_pcb.pcb[i].status == 1 || info_pcb.pcb[i].status == 3) {
                    exitCount++;
                } 
            }
            printf("Total exited process: %d\n", exitCount);
        } else if(!strcmp(tokens[1], "2")) {
            int runningCount = 0;
            for (int i = 0; i < info_pcb.in; i++) {
                if (info_pcb.pcb[i].status == 2 || info_pcb.pcb[i].status == 4) {
                    runningCount++;
                } 
            }
            printf("Total running process: %d\n", runningCount);
        } else {
            fprintf(stderr, "Wrong command\n");
        }
        return;
    } 

    // If last string is "&" remove it from the arguments
    int w = 1;
    if (!strcmp(tokens[tokenNo - 2], "&")) {
        tokens[tokenNo - 2] = NULL;
        w = 0;
    }

    pid_t curr_pid = getpid();
    // Fork child process
    pid_t childpid = fork();

    if (childpid == 0) {
        // Attach to shared memory in child process
        shm = (int*)shmat( shmid, NULL, 0);
        PCB_wrapper child_pcb = * (PCB_wrapper*) shm[1];
        while(0 == 0) {
            if (shm[0] == 0) {
                shm[0] == 1;
                add_process(child_pcb, childpid, 2);
                shm[0] == 0;
                break;
            }
        }
        execvp(command, tokens);
        //returns if exec throws error
        fprintf(stderr, "%s not found\n", command);
        while(0 == 0) {
            if (shm[0] == 0) {
                shm[0] == 1;
                exit_process(child_pcb, childpid, 1);
                shm[0] == 0;
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
    PCB_wrapper final_pcb = * (PCB_wrapper*) shm[1];
    for(int i = 0; i < final_pcb.in; i++) { 
        final_pcb.pcb[i].status = 3; //terminated
        final_pcb.pcb[i].exitCode = 143; //SIGTERM
        if ((final_pcb.pcb[i].pid != curr_pid) && ((final_pcb.pcb[i].status == 4) || (final_pcb.pcb[i].status == 2))) {
            printf("Killing [%d]", final_pcb.pcb[i].pid);
            kill(final_pcb.pcb[i].pid, SIGTERM);
        }
    }
    printf("Killing [%d]\n", curr_pid);
    printf("\nGoodbye\n");
    kill(curr_pid, SIGTERM);
}
