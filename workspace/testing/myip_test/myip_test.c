#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define MYIP_OFFSET 0x40008000U
struct myip_regs {
	volatile uint32_t samples;
	volatile uint32_t rate;
	volatile uint32_t scale;
	volatile uint32_t status_control;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct myip_regs* regs;

	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, MYIP_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	fprintf(stdout, "regs->status: %08x\n", regs->status_control);
	regs->status_control = 0x1000000U;
	fprintf(stdout, "regs->status: %08x\n", regs->status_control);

	munmap(regs, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
