#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define GPIO_OFFSET 0x40004000U
struct gpio_regs {
	uint32_t data;
	uint32_t tri;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct gpio_regs* regs;

	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, GPIO_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	fprintf(stdout, "regs->status: %08x\n", regs->tri);

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
