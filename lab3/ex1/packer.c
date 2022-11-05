#include "packer.h"
#include "semaphore.h"
#include <stdlib.h>
#include <stdio.h>

// You can declare global variables here
sem_t red_sem; //red = 1
sem_t redlog_sem;
sem_t green_sem; //green = 2
sem_t greenlog_sem;
sem_t blue_sem; //blue = 3
sem_t bluelog_sem;

int red[2];
int redind = 0;

int green[2];
int greenind = 0;

int blue[2]; 
int blueind = 0;

void packer_init(void)
{
    fflush(stdout);
    printf("packinit called\n");
    // Write initialization code here (called once at the start of the program).
    sem_init(&red_sem, 0, 1);
    sem_init(&redlog_sem, 0, 0);

    sem_init(&green_sem, 0, 1);
    sem_init(&greenlog_sem, 0, 0);

    sem_init(&blue_sem, 0, 1);
    sem_init(&bluelog_sem, 0, 0);
}

void packer_destroy(void)
{
    // Write deinitialization code here (called once at the end of the program).
    sem_destroy(&red_sem);
    sem_destroy(&green_sem);
    sem_destroy(&blue_sem);
    // free(red_sem);
    // free(green_sem);
    // free(blue_sem);
}

int pack_ball(int colour, int id)
{
    printf("pack_ball called\n");
    // Write your code here.
    if (colour == 1) {
        //Log id protected by [colour]_sem
        printf("waiting for red\n");
        sem_wait(&red_sem);
        red[redind] = id;
        redind++;
        if (redind == 2) {
            sem_post(&redlog_sem);
        }
        sem_post(&red_sem);

        sem_wait(&redlog_sem);
        if (red[0] != id) {
            return red[0];
        } else {
            return red[1];
        }
        sem_post(&redlog_sem);
    } else if (colour == 2) {
        //Log id protected by [colour]_sem
        printf("waiting for green\n");
        sem_wait(&green_sem);
        green[greenind] = id;
        greenind++;
        if (greenind == 2) {
            sem_post(&greenlog_sem);
        }
        sem_post(&green_sem);

        sem_wait(&greenlog_sem);
        if (green[0] != id) {
            return green[0];
        } else {
            return green[1];
        }
        sem_post(&greenlog_sem);
    } else {
        //Log id protected by [colour]_sem
        printf("waiting for blue\n");
        sem_wait(&blue_sem);
        blue[blueind] = id;
        blueind++;
        if (blueind == 2) {
            sem_post(&bluelog_sem);
        }
        sem_post(&blue_sem);

        sem_wait(&bluelog_sem);
        if (blue[0] != id) {
            return blue[0];
        } else {
            return blue[1];
        }
        sem_post(&bluelog_sem);
    }
    return 0;
}