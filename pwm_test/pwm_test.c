#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define TIMER_OFFSET 0x40002000U
#define MAX_COUNT 0xFFFFFFFFU

struct timer_regs {
	uint32_t tcsr0;
	uint32_t tlr0;
	uint32_t tcr0;
	uint8_t reserved_0[4];
	uint32_t tcsr1;
	uint32_t tlr1;
	uint32_t tcr1;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct timer_regs* regs;

	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, TIMER_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	pwm_period = (MAX_COUNT - tlr0 + 2) * AXI_CLOCK_PERIOD;
	pwm_high_time = (MAX_COUNT - tlr1 + 2) * AXI_CLOCK_PERIOD;

	fprintf(stdout, "regs->status: %08x\n", regs->tcsr1);

	for(uint32_t i = 0; i < 256; i++) {
		regs->data = i & 0xf;
		usleep(250000);
	}

	munmap(regs, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
