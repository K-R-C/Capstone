#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MYIP_OFFSET 0

struct mmregs {
	volatile unsigned int samples;			//reg1
	volatile unsigned int rate;				//reg2
	volatile unsigned int scale;			//reg3
	volatile unsigned int status_control;	//reg4
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct mmregs* regs;
	extern uint32_t binary_data_start[];
	extern uint32_t* binary_data_end;


	if((fd = open("/dev/audioctl0", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MYIP_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	regs->samples = 100;
	regs->rate = /* 4096 */ 1024;
	regs->status_control = 2;

	for(unsigned int i = 440; i < 4096*2; i += 4) {
		regs->rate = i;
		usleep(500);
	}

	for(unsigned int i = 4096*2; i >= 440; i -= 4) {
		regs->rate = i;
		usleep(500);
	}


	munmap((void*)regs, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
