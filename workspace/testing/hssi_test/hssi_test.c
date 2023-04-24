#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define HSSI_OFFSET 0x41000000U


struct mmregs {
	volatile unsigned int tx_data;
	volatile unsigned int rx_data;
	volatile unsigned int csr;
};

int main(void) {
	// static const unsigned int msg[] = {0xaaaaaaaa, 0x55555555, 0xcccccccc, 0x33333333};
	static const unsigned int msg[] = {0xfe919e49, 0x505ba6f6, 0x6bc03baf, 0x7abd5bad};
	size_t msg_index = 0, trial_count = 0;
	unsigned int rx_data;
	int fd, ret = EXIT_FAILURE;
	struct mmregs* regs;

	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}

	regs = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, HSSI_OFFSET);

	if(regs == MAP_FAILED) {
		perror("mmap()");
		goto err_mmap;
	}

/* set then clear the reset bit */
	//regs->csr |= (1 << 3);
	
/*
	while(!(regs->csr & (1 << 22))) { // check for clocks to settle
		;
	}
	regs->csr |= (1 << 1); // enable TX
	fprintf(stderr, "Clock status bit is: %08x\n", regs->csr &= (1 << 22));
	regs->csr |= (1 << 0);
	fprintf(stderr, "TX Ready bit is: %08x\n", regs->csr &= (1 << 19));
	regs->csr &= ~(1 << 0);
	fprintf(stderr, "After toggling reset, TX Ready bit is: %08x\n", regs->csr &= (1 << 19));
	
	regs->tx_data = 0x55555555;
	regs->csr |= (1 << 2); // start transmission
	
	while(!(regs->csr & (1 << 16))) { // check for TX Done
		;
	}
	fprintf(stderr, "TX Done bit is: %08x\n", regs->csr &= (1 << 16));
*/
regs->csr &= ~(1 << 0);
regs->csr &= ~(1 << 1);
regs->csr &= ~(1 << 2);
regs->csr &= ~(1 << 3);
regs->csr &= ~(1 << 4);
regs->csr &= ~(1 << 5);
regs->csr &= ~(1 << 6);
	while(!(regs->csr & (1 << 22))) { // check for clocks to settle
		;
	}
	//fprintf(stderr, "clock locked\n");
	regs->csr |= (1 << 1);		//TX Enable
	regs->csr |= (1 << 4); 		//RX Enable
			
	regs->csr |= (1 << 6);	// clear RX
	regs->csr &= ~(1 << 6);
	//fprintf(stderr, "TX Enabled\n");

	while(1) {
		
		trial_count ++;
		while(!(regs->csr & (1 << 19))) {
			/* wait for transmitter*/ ;
		}
		//fprintf(stderr, "TX Ready\n");
		regs->tx_data = msg[msg_index]; /* message to send */
		
		while(!(regs->csr & (1 << 23))) {
			; //wait for RX Ready
		}
		regs->csr |= (1 << 2);			  /* start transmission */
		while(!(regs->csr & (1 << 16))) {
			; //wait for TX Done
		}
		//fprintf(stderr, "TX Done\n");
		regs->csr &= ~(1 << 2);

		while(!(regs->csr & (1 << 20))) {
			; //wait for RX Data Good
		}

		rx_data = regs->rx_data;
		
		if(msg[msg_index] != rx_data) {
			fprintf(stderr, "RX/TX failure: got %08x, expected %08x\n", rx_data, msg[msg_index]);
			fprintf(stderr, "on trial count %zu\n", trial_count);
			break;
		}
		msg_index++;
		msg_index %= (sizeof(msg)/sizeof(msg[0]));	
		//fprintf(stderr, "sent %08x\n", msg[msg_index]);
	}

	munmap((void*)regs, 4096);
	ret = EXIT_SUCCESS;
	
err_mmap:
	close(fd);
	return ret;
}
