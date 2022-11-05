#include "zc_io.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <sys/mman.h>
#define _GNU_SOURCE 1

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
    int fd = open(path,   O_RDWR | O_CREAT, 0666);
    if (fd == -1) {
        perror("Cannot open file\n");
        return NULL;
    }

    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat failed\n");
        return NULL;
    }

    char* RAM_file = (char*) mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);

    zc_file* file = (zc_file*) malloc(sizeof(zc_file));
    file->fileDesc = fd;
    file->fileSize = sb.st_size;
    file->memoryFile = RAM_file;
    file->offset = 0;

    // printf("fd:%d\n", fd);
    // printf("fileSize:%zu\n", sb.st_size);
    // printf("char pointer:%p\n", RAM_file);

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
        printf("if\n");
        ftruncate(file->fileDesc, file->offset + size);
        file->memoryFile = (char*) mremap(
            (void*)file->memoryFile, file->fileSize, file->offset + size, 0);
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
    // To implement
    return -1;
}

/**************
 * Exercise 4 *
 **************/

int zc_copyfile(const char *source, const char *dest) {
    // To implement
    return -1;
}
