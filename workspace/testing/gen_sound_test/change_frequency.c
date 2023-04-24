#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MYIP_OFFSET 0x40008000U

#define RATE_MAX (4096 + 6*1024)
#define RATE_MIN (4096 - 2*1024)
#define RATE_DELTA 16

struct mmregs {
	volatile unsigned int samples;
	volatile unsigned int rate;
	volatile unsigned int scale;
	volatile unsigned int status_control;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct mmregs* regs;
	unsigned int rate = 4096;
	unsigned int direction = 0;
	extern uint32_t binary_data_start[];
	extern uint32_t* binary_data_end;


	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MYIP_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	regs->samples = 100;
	regs->rate = 4096;
	regs->status_control = 1;
	regs->status_control = 2;

	while(1) {
		if(direction) {
			rate -= RATE_DELTA;
			if(rate <= RATE_MIN) {
				direction = 0;
			}
		} else {
			rate += RATE_DELTA;
			if(rate >= RATE_MAX) {
				direction = 1;
			}
		}

		regs->rate = rate;
		// regs->status_control = 1;
		// regs->status_control = 2;
		usleep(2500);
		
	}


	munmap((void*)regs, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
