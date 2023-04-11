#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BRAM_OFFSET 0x42000000U
// 		REGS_OFFSET = 0x40008000U

int main(void) {
	int fd, fd_2, ret = EXIT_FAILURE;
	volatile uint32_t* bram;
	volatile uint32_t* bram_2;
	extern uint32_t binary_data_start[];
	extern uint32_t binary_data_end[];


	//if((fd = open("/dev/audioctl0", O_RDWR)) < 0) {
		//perror("open()");
		//return EXIT_FAILURE;
	//}
	
	fd_2 = open("/dev/mem", O_RDWR);

	//bram = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
	bram_2 = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd_2, BRAM_OFFSET);

	if(bram_2 == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}
	
	//bram[0] = 0xaa55aa55;

	//printf("bram mapped at: %p\n", bram);

	for(size_t i = 0; binary_data_start + i < binary_data_end; i++) {
		printf("offset %04x\n", i);
		*(bram_2 + i) = *(binary_data_start + i);
	}
	
	/*
	printf("\ndump\n-------\n");

	for(size_t i = 0; i < 4096/4; i++) {
		printf("%08x ", *(bram + i));
	}
	printf("\n");
	
	printf("\ndump\n-------\n");
	for(size_t i = 0; i < 4096/4; i++) {
		printf("%08x ", *(bram_2 + i));
	}
	printf("\n");
	*/

err_ok:
	munmap((void*)bram_2, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
