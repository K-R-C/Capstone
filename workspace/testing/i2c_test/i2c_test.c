#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define I2C_OFFSET 0x40000000U
struct i2c_regs {
	uint8_t padding_bytes[0x100];
	volatile uint32_t control;
	volatile uint32_t status;
};

int main(void) {
	int fd, ret = EXIT_FAILURE;
	struct i2c_regs* regs;

	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, I2C_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

	fprintf(stdout, "regs->status: %08x\n", regs->status);

	munmap(regs, 4096);
	ret = EXIT_SUCCESS;
err_mmap:
	close(fd);
	return ret;
}
