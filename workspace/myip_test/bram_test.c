#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define BRAM_OFFSET 0x42000000U
struct i2c_regs {
	volatile uint32_t samples;
	volatile uint32_t rate;
	volatile uint32_t scale;
	volatile uint32_t status_control;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	volatile uint32_t* bram;

	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	bram = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, BRAM_OFFSET);

	if(bram == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	for(size_t i = 0; i < 4096/4; i++) {
		*(bram + i) = i;
		printf("%08x ", *(bram + i));
	}
	printf("\ndump\n-------\n");

	for(size_t i = 0; i < 4096/4; i++) {
		printf("%08x ", *(bram + i));
	}
	printf("\n");

	munmap((void*)bram, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
