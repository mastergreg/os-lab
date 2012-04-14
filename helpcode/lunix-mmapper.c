/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : lunix-mmapper.c
* Creation Date : 14-04-2012
* Last Modified : Sat 14 Apr 2012 11:10:22 AM EEST
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
    int fd = open(argv[1], 0);
    int i;
    unsigned int *map;

    map = mmap(NULL, 42, PROT_READ, MAP_PRIVATE, fd, 0);

    for (i = 0 ; i < 42 ; ++i) {
        printf("state type: %u\n", map[i]);
    }

    return 0;
}
