#define _GNU_SOURCE 

#include "zc_io.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <error.h>


// The zc_file struct is analogous to the FILE struct that you get from fopen.
struct zc_file {
    size_t fileSize;
    int fileDesc;
    char* memoryFile;
    size_t offset; 
};



/**************
 * Exercise 1 *
 **************/

zc_file* zc_open(const char *path) {
    int fd = open(path, O_RDWR | O_CREAT, 0666); //change
    if (fd == -1) {
        perror("Cannot open file\n");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat failed\n");
        return NULL;
    }

    char* RAM_file;

    // mremap fails when attempting to remap a memory space with size 0
    if (sb.st_size == 0) {
        RAM_file = (char*) mmap(NULL, 1, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    } else {
        RAM_file = (char*) mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    }

    

    zc_file* file = (zc_file*) malloc(sizeof(zc_file));
    file->fileDesc = fd;
    file->fileSize = sb.st_size;
    file->memoryFile = RAM_file;
    file->offset = 0;

    return file;
}



int zc_close(zc_file *file) {
    close(file->fileDesc);
    int res = munmap((*file).memoryFile, (*file).fileSize);
    free(file);
    return res;
}

const char* zc_read_start(zc_file *file, size_t *size) {
    if ((file->offset + *size) > file->fileSize) {
        *size = file->fileSize - file->offset;
        // file->memoryFile = (char*) mremap((void*)file->memoryFile, file->fileSize, file->offset + *size, 0);
        // file->fileSize = file->offset + *size;
    }
    const char* res = file->memoryFile + file->offset;
    file->offset = file->offset + *size;
    return res;
}

void zc_read_end(zc_file *file) {
    
}

/**************
 * Exercise 2 *
 **************/

char* zc_write_start(zc_file *file, size_t size) {
    if ((file->offset + size) > file->fileSize) {
        // printf("if\n");
        char* newLoc = malloc(file->offset + size);
        
        ftruncate(file->fileDesc, file->offset + size);
        // printf("ftruncate done\n");
        // printf("old size: %zu\n", file->fileSize);
        // printf("new size: %zu\n", file->offset + size);

        file->memoryFile = (char*) mremap(
            (void*)file->memoryFile, file->fileSize, file->offset + size, MREMAP_MAYMOVE);
        if (file -> memoryFile == MAP_FAILED) {
            perror("mremap failed");
        } 
        // else {
        //     printf("mremap done\n");
        // }
        file->fileSize = file->offset + size;
    }

    const char* res = file->memoryFile + file->offset;
    file->offset = file->offset + size;
    return res;
}

void zc_write_end(zc_file *file) {
    // To implement
    
}

/**************
 * Exercise 3 *
 **************/

off_t zc_lseek(zc_file *file, long offset, int whence) {
    if (whence == SEEK_SET) {
        file->offset = (size_t) offset;
        return file->offset;
    } else if (whence == SEEK_CUR) {
        file->offset = (size_t) file->offset + offset;
        return file->offset; 
    } else if (whence == SEEK_END){
        file->offset = (size_t) file->fileSize + offset;
    } else {
        return -1;
    }
}

/**************
 * Exercise 4 *
 **************/

int zc_copyfile(const char *source, const char *dest) {
    // To implement
    return -1;
}
