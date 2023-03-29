#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define IOREGS_OFFSET	0x40008000U

#define USE_DEV_MEM		(1 << 0)

struct regs { 								//TODO: change this
	volatile uint32_t rate;
	volatile uint32_t scale;
	volatile uint32_t samples;
	volatile uint32_t status_control;
};

int main(int argc, char** argv) {
	const char* chardev = "/dev/audioctl0";
	off_t offset = 0;
	int fd, ret = EXIT_FAILURE, opt;
	struct regs* ioregs;

	while((opt = getopt(argc, argv, "mc:")) != -1) {
		switch(opt) {
			case 'm':
				chardev = "/dev/mem";
				offset = IOREGS_OFFSET;
				break;
			case 'c':
				chardev = optarg;
				offset = 0;
				break;
			default:
				fprintf(stderr, "usage: %s [-m] [-c /dev/device_node]\n", argv[0]);
				return EXIT_FAILURE;
		}
	}

	if((fd = open(chardev, O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}
	
	ioregs = (struct regs*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, offset);

	if(ioregs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}
	
	printf("using device %s\n", chardev);
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
