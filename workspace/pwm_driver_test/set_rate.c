#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <getopt.h>
#include <unistd.h>


#define DEFAULT_CHARDEV "/dev/audioctl0"

static inline int set_rate(int fd, unsigned int rate) {
	return ioctl(fd, _IOW('A', 'r', int), rate);
}


int main(int argc, char** argv) {
	char* chardev = NULL;
	int opt, fd, ret = EXIT_FAILURE;


	static const struct option long_opts[] = {
			{ "device",		required_argument,	NULL,	'f' },
			{ 0, 0, NULL, 0 }
		};

	while((opt = getopt_long(argc, argv, "f:", long_opts, (int*)NULL)) >= 0) {
		switch(opt) {
			case 'f':
				free(chardev);
				chardev = strdup(optarg);
				break;
			default:
				break;
		}
	}


	if(!chardev) {
		chardev = DEFAULT_CHARDEV;
	}

	if((fd = open(chardev, O_RDWR)) < 1) {
		perror("open()");
		goto err_out;
	}

	ret = set_rate(fd, 100);

//err_ioctl:
	//close(fd);
err_out:
	return ret;
}
