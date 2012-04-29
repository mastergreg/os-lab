/* -.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.-.
* File Name : lunix-ioctl-reader.c
* Creation Date : 14-04-2012
* Last Modified : Sun 29 Apr 2012 06:54:30 PM EEST
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

#include "lunix.h"
#include "lunix-chrdev.h"

int main(int argc, char **argv)
{
    if(argc != 2) {
        printf("Usage: %s device\n",argv[0]);
        exit(EXIT_FAILURE);
    }
    int fd = open(argv[1], 0);
    int i;
    char buf[1024];
    buf[1023]='\0';
    unsigned int *map;

    ioctl(fd, LUNIX_IOC_RAW, NULL);

    while (read(fd, buf, 1023)) {
        printf("raw type: %X\n", buf[0]);
    }

    return 0;
}
