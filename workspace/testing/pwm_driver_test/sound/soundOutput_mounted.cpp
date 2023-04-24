#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include <unistd.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <queue>
#include <cmath>

#include <fstream>
#include <string>
#include <iostream>

#define STATUS_CONTROL_RESET			(1 << (0))
#define STATUS_CONTROL_RESET_OFFSET		(0)
#define STATUS_CONTROL_RESET_MASK		(1 << (0))
#define STATUS_CONTROL_ENABLE			(1 << (1))
#define STATUS_CONTROL_ENABLE_OFFSET	(1)
#define STATUS_CONTROL_ENABLE_MASK		(1 << (1))

using namespace std; 

//string number;

struct regs {
	volatile uint32_t samples;
	volatile uint32_t rate;
	volatile uint32_t scale;
	volatile uint32_t status_control;
};

void dump_regs(struct regs* ioregs);
unsigned int do_rate(unsigned int rate);

int main(int argc, char* argv[]) {		// this is how the front end code for our project is going to look like 
	int fd, ret = EXIT_FAILURE;
	struct regs* ioregs;
	uint32_t rate, scale;
	//uint32_t pmodAmp;
	//uint32_t pmodFreq;
	
	if((fd = open("/dev/mem", O_RDWR)) < 0) {
		perror("open()");
		return EXIT_FAILURE;
	}
	
	ioregs = (struct regs*)mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0x40008000);

	if(ioregs == MAP_FAILED) {
		perror("mmap()");
		close(fd);
		return EXIT_FAILURE;
	}
	
	dump_regs(ioregs);
	
	ioregs->status_control = STATUS_CONTROL_RESET;	/* reset the core */
	ioregs->status_control = 0;
	
	ioregs->samples = 100;
	ioregs->status_control = STATUS_CONTROL_ENABLE;				/* enable the core */

	printf("\nafter setup\n");
	dump_regs(ioregs);
    
    ifstream pmodAmp;
    ifstream pmodFreq;
    
    //ioregs->rate = 440;

		
    while(true) {
		//pmodAmp.open("/sys/bus/iio/devices/iio:device1/in_proximity_raw");
		//pmodAmp >> scale;
		//ioregs->scale = scale;
		//pmodAmp.close();

		pmodFreq.open("/sys/bus/iio/devices/iio:device2/in_proximity_raw");
		pmodFreq >> rate;
		pmodFreq.close();
		
		rate = do_rate(rate/8);
		ioregs->rate = rate;
		//pmodFreq.close();

		std::cout << (rate) << std::endl;
		//usleep(250);
		// cout<<"amplitude:  "<< ioregs->scale <<"\nfrequency:  "<< ioregs->rate <<endl;
    }

    return 0;

}

void dump_regs(struct regs* ioregs) {
	printf(
			"register dump"			"\n"
			"--------------"		"\n"
			"rate:\t\t%08x"			"\n"
			"scale:\t\t%08x"		"\n"
			"samples:\t%08x"		"\n"
			"status:\t\t%08x"		"\n",
			ioregs->rate, ioregs->scale,
			ioregs->samples, ioregs->status_control);
}

#define RATE_TOP	(7000/8)
#define	RATE_BOTTOM	(4100/8)
uint32_t do_rate(uint32_t rate) {
	if(rate > RATE_TOP) {
		rate = RATE_TOP;
	} else if(rate < RATE_BOTTOM) {
		rate = RATE_BOTTOM;
	}
	
	int64_t rate_new = rate;
	
	/* scale */
	rate_new = 8*(1412 * rate_new)/1000 - 1694; //10519;
	return rate_new;
}
