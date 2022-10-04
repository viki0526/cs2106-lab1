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
#include<fcntl.h> 

struct PCBTable1
{
    pid_t pid;
    pid_t ppid;
    int status;   // 4: Stopped, 3: Terminating, 2: Running, 1: exited
    int exitCode; // -1 not exit, else exit code status
};

typedef struct {
    struct PCBTable1 pcb[200];
    int in;
    int flag;
} PCB_wrapper;
int shmid;
PCB_wrapper* shm;

void add_process(PCB_wrapper* pcb_wrapper, pid_t p, int stat, pid_t pp) {
    int i = (*pcb_wrapper).in;
    while(1) {
        if ((*pcb_wrapper).flag == 0) {
            (*pcb_wrapper).flag = 1;
            ((*pcb_wrapper).pcb[i]).pid = p;
            ((*pcb_wrapper).pcb[i]).status = stat;
            ((*pcb_wrapper).pcb[i]).exitCode = -1;
            ((*pcb_wrapper).pcb[i]).ppid = pp;
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

pid_t getpp(PCB_wrapper* pcb_wrapper, pid_t p) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        if (((*pcb_wrapper).pcb[i].pid == p)) {
            return (*pcb_wrapper).pcb[i].ppid;
        }
    }
    return 0;
}

void print_PCB(PCB_wrapper* pcb_wrapper) {
    for(int i = 0; i < (*pcb_wrapper).in; i++) {
        printf("pid: %d\n", ((*pcb_wrapper).pcb[i]).pid);
        printf("ppid: %d\n", ((*pcb_wrapper).pcb[i]).ppid);
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
        shm[0].pcb[i].ppid = -1;
        shm[0].pcb[i].status = -1;
        shm[0].pcb[i].exitCode = -1;
    }
    shm[0].flag = 0; //Use shm[0].flag as a flag for whether the PCB is free(0) or being modified(1);
    // printf("shm[0].flag\n");
    return;
}

void my_process_command(size_t num_tokens, char **tokens) {
    int tokenNo = (int) num_tokens;
    char *cmd_tokens[100];

    int count = 0; 
    for (int i = 0; i < tokenNo; i++) {
        if (tokens[i] == NULL) {
            cmd_tokens[count] = NULL;
            my_process_command_wrapped((size_t) count + 1, cmd_tokens);
            break;
        } else if (!strcmp(tokens[i], ";")) {
            // printf(";\n");
            cmd_tokens[count] = NULL;
            my_process_command_wrapped((size_t) count + 1, cmd_tokens);
            count = 0;
        } else {
            // printf("I'm at else\n");
            cmd_tokens[count] = tokens[i];
            count++;
        } 
    }
    return;
}

void my_process_command_wrapped(size_t num_tokens, char **full_tokens) {
    int tokenNo = (int) num_tokens;
    char* tokens[100]; //space for array of 100 tokens
    int pipeIndex = 0;
    //Fill up cmd_tokens with the command and arguments and null-terminate
    for (int i = 0; i < num_tokens; i++) {
        if (full_tokens[i] == NULL) {
            tokens[i] = NULL;
            break;
        } else if (!strcmp(full_tokens[i], "<") || !strcmp(full_tokens[i], ">") || !strcmp(full_tokens[i], "2>")) {
            tokens[i] = NULL;
            tokenNo = i + 1;
            pipeIndex = i;
            break;
        } else {
            tokens[i] = full_tokens[i];
        }
    }

    // for (int i = 0; i < tokenNo; i++) {
    //     if (tokens[i] != NULL) {
    //         printf("%s\n", tokens[i]);
    //     }
    // }

    //use tokens and tokenNo
    char* command = tokens[0];

    //Strings inputFile, outputFile and errorFile are assigned if present in the command
    char* inputFile = NULL;
    char* outputFile = NULL;
    char* errorFile = NULL;
    if (pipeIndex > 0) {
        for (int i = pipeIndex; i < num_tokens; i++) {
            if (full_tokens[i] == NULL) {
                break;
            } else if (!strcmp(full_tokens[i], "<") && full_tokens[i + 1] != NULL) {
                inputFile = full_tokens[i + 1];
            } else if (!strcmp(full_tokens[i], ">") && full_tokens[i + 1] != NULL) {
                outputFile = full_tokens[i + 1];
            } else if (!strcmp(full_tokens[i], "2>") && full_tokens[i + 1] != NULL) {
                errorFile = full_tokens[i + 1];
            }
        }
    }
   


    pid_t parentpid = getpid();

    if (!strcmp(command, "wait")) {
        if (tokens[1] == NULL) {
            fprintf(stderr, "Wrong command\n");
        } else {
            pid_t pid_arg = strtol(tokens[1], NULL, 0);
            if (running(&shm[0], pid_arg)) {
                int status;
                printf("Waiting for %d\n", pid_arg);
                waitpid(getpp(&shm[0], pid_arg), &status, 0);
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
                printf("Total terminating process: %d\n", terminatingCount);
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

    // Create fd and pipe to pass grandchild pid to parent 
    pid_t fd[2];
    if (pipe(fd) == -1) {
        perror("Error occured with opening pipe");
        exit(1);
    } 

    // Fork child process
    pid_t childpid = fork();

    if (childpid == 0) {
        pid_t childpid = getpid();
        pid_t grandchildpid = fork();

        //Run exec in grandchild process
        if (grandchildpid == 0) {
            if (inputFile != NULL) {
                int infd = open(inputFile, O_RDONLY, S_IRUSR|S_IWUSR);
                if (infd == -1) {
                    fprintf(stderr, "%s does not exist\n", inputFile);
                    exit(1);
                } else {
                    dup2(infd, 0);
                }
            }
            if (outputFile != NULL) {
                int outfd = open(outputFile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
                if (outfd == -1) {
                    perror("Error stdout opening file");
                    exit(1);
                } else {
                    dup2(outfd, 1);
                }
            }
            if (errorFile != NULL) {
                int errfd = open(errorFile, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
                if (errfd == -1) {
                    perror("Error stderr opening file");
                    exit(1);
                } else {
                    dup2(errfd, 2);
                }
            }
            execvp(command, tokens);
            exit(127); //command not found
        }
        //Wait and log exitcode in child process
        else {
            // Attach to shared memory
            PCB_wrapper* shm1 = shmat(shmid, NULL, 0);
            add_process(&shm1[0], grandchildpid, 2, getpid());
            // Write grandchild pid to pipe
            close(fd[0]);
            write(fd[1], &grandchildpid, sizeof(pid_t));
            close(fd[1]);

            //Wait for grandchild to exit
            int status;
            if (waitpid(grandchildpid, &status, 0) == -1) {
                perror("waitpid failed");
                exit(1);
            }
            //Log exit/signal code to PCB
            else {
                if (WIFEXITED(status)) {
                    const int exit_code = WEXITSTATUS(status);
                    if (exit_code == 127) {
                        fprintf(stderr, "%s not found\n", command);
                    } 
                    exit_process(&shm1[0], grandchildpid, exit_code);
                    exit(0);
                } else if(WIFSIGNALED(status)) {
                    exit_process(&shm1[0], grandchildpid, WTERMSIG(status));
                    exit(0);
                } else {
                    perror("Undefined exit");
                }
            }
        }   
    } else {
        // Block current process and wait for child process to exit if wait == 1
        // Otherwise print "Child [%d] in background" and exit
        if(w == 1) {
            wait(NULL);
        } else {
            close(fd[1]);
            pid_t message;
            read(fd[0], &message, sizeof(pid_t));
            close(fd[0]);
            printf("Child [%d] in background\n", message);
        }
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
