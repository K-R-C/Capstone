#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

struct regs {
	volatile uint32_t rate;
	volatile uint32_t scale;
	volatile uint32_t samples;
	volatile uint32_t status_control;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct regs* ioregs;
	volatile uint32_t* bram_2;

	if((fd = open("/dev/audioctl0", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}
	
	ioregs = (struct regs*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

	if(ioregs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}
	
	printf(
			"register dump"			"\n"
			"--------------"		"\n"
			"rate:\t\t%08x"			"\n"
			"scale:\t\t%08x"		"\n"
			"samples:\t%08x"		"\n"
			"status:\t\t%08x"		"\n",
			ioregs->rate, ioregs->scale, ioregs->samples, ioregs->status_control);
	
	ret = EXIT_SUCCESS;
	
err_ok:
	munmap((void*)ioregs, 4096);
err_mmap:
	close(fd);
	return ret;
}
