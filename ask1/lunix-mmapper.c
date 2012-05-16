/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : lunix-mmapper.c
* Creation Date : 14-04-2012
* Last Modified : %+
* Created By : Greg Liras <gregliras@gmail.com>
_._._._._._._._._._._._._._._._._._._._._.*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "lunix-chrdev.h"

int main(int argc, char **argv)
{
    if(argc != 2) {
        printf("Usage: %s device\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    int fd = open(argv[1], O_RDWR | O_CREAT | O_TRUNC, (mode_t)0600);

    unsigned int *map1;

    map1 = mmap(NULL, 1, PROT_READ, MAP_SHARED, fd, 0);
    if (map1 == MAP_FAILED) {
        close(fd);
        perror("Error mmapping the file");
        exit(EXIT_FAILURE);
    }

    printf("state type: %u\n", map1[0]);

    return 0;
}
