#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <sys/mman.h>

#include "../lunix.h"
#include "../lunix-lookup.h"

/*
 * The mmap method of this lunix driver returns the page with the raw value, timestamp and last update 32bit indicator sequence.
 * In order to use mmap on lunixdevice nodes, you have to include lunix.h (for the struct definition),
 * and lunix-lookup.h if you want to find the correspondence between raw-representing value.
 * Observe how a sample implementation is done below.
 */

int main (int argc, char *argv[]) {
	fprintf(stdout, "This script opens the given device for reading and maps it to a page in the process page table.\n");
	
	if (argc!=2) {
		fprintf(stderr, "Usage: %s <device_node_path>.\n", argv[1]);
		return 1;
	}
	
	long *real_value=NULL;
	fprintf(stdout, "Locating the proper lookup table for the inserted node.\n");
	if (strstr(argv[1], "batt"))
		real_value=lookup_voltage;
	else if (strstr(argv[1], "light"))
		real_value=lookup_light;
	else if (strstr(argv[1], "temp"))
		real_value=lookup_temperature;
	else
		real_value=NULL;
	
	fprintf(stdout, "Opening file.\n");
	int file_d=open(argv[1], O_RDONLY);
	if (file_d==-1) {
		fprintf(stderr, "File %s could not be opened (value %d). Terminating.\n", argv[1], errno);
		return 1;
	}
	
	char check_working[20]="";
	ssize_t rrv=0;
	fprintf(stdout, "Initiating read of 10 bytes from file.\n");
	if ((rrv=read(file_d, check_working, 10))==-1) {
		fprintf(stderr, "Read from file failed with value %d.\n", errno);
		return 1;
	}
	fprintf(stdout, "The read string was of size %d : '%s'.\n", rrv, check_working);
	
	struct lunix_msr_data_struct *buffer_page=NULL;
	fprintf(stdout, "Mapping the file on an OS-specified address.\n");
	buffer_page=(struct lunix_msr_data_struct *)mmap(NULL, sysconf(_SC_PAGE_SIZE), PROT_READ, MAP_SHARED, file_d, 0);
	if (buffer_page==MAP_FAILED) {
		fprintf(stderr, "Mapping of file to buffer failed with value %d.\n", errno);
		return 1;
	}
	fprintf(stdout, "Mapping returned with address %p.\n", buffer_page);
    close(file_d);
	
	size_t i=0;
	fprintf(stdout, "Printing the process' memory pages.\n");
	char procgarb[40];	snprintf(procgarb, 40, "cat /proc/%ld/maps", (long)getpid()); system(procgarb);
//	system("cat /proc/self/maps");		//Doesn't work: prints cat's mappings.
	sleep(1);
	
	while (i<20) {
		uint32_t raw_data=buffer_page->values[0];
		fprintf(stdout, "Buffer (raw value) is %u \t and lookup (cooked value) is %ld.%ld.\n", raw_data, 
						(real_value[raw_data])/1000 , (real_value[raw_data])%1000);
		usleep(0.5);
		i++;
	}
	
	if (munmap((void *)buffer_page, sysconf(_SC_PAGE_SIZE))) {
		fprintf(stderr, "Error while unmapping the file with value %d.\n", errno);
		return 1;
	}
	
	return 0;
}

