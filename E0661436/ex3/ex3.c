/*************************************
 * Lab 1 Exercise 3
 * Name: Vikrant Prakash
 * Student No: A0227630L
 * Lab Group: 3
 *************************************/

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>

#include "function_pointers.h"
#include "node.h"

// The runner is empty now! Modify it to fulfill the requirements of the
// exercise. You can use ex2.c as a template

// DO NOT initialize the func_list array in this file. All initialization
// logic for func_list should go into function_pointers.c.

// Macros
#define SUM_LIST 0
#define INSERT_AT 1
#define DELETE_AT 2
#define SEARCH_LIST 3
#define REVERSE_LIST 4
#define RESET_LIST 5
#define LIST_LEN 6
#define MAP 7

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Error: expecting 1 argument, %d found\n", argc - 1);
        exit(1);
    }
    FILE * fp;
    fp = fopen(argv[1], "r");
    if (fp == NULL) {
        printf("Error: file does not exist.");
        exit(1);
    }
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
        if (line[0] == '\n' | line[0] == '\0' | line[0] == ' ') 
            break;
        
        int j = (int) read/2;

        for (int i = 0; i < j; i++) {
            char* num = strtok(line, " \n\0");
        }
        



        printf("Retrieved line of length %zu:\n", read);
        printf("%s", line);
    }

    fclose(fp);

    // We read in the file name provided as argument

    // Update the array of function pointers
    // DO NOT REMOVE THIS CALL
    // (You may leave the function empty if you do not need it)
    update_functions();

    // Rest of code logic here
}


