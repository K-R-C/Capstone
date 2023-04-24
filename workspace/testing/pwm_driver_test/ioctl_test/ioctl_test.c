#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <getopt.h>
#include <unistd.h>

// #include <segment_driver_ioctls.h>

#define DEFAULT_CHARDEV "/dev/audioctl0"

#define PGM_LAUNCH(pgm, arg) ({ \
	int retval = 0; \
	if(!strcmp(pgm.name, prog_name)) { \
		retval = pgm.fn(fd, &arg); \
	} \
	retval; \
	})

typedef int (*fn_ptr)(int, ...);

static inline int set_rate(int fd, ...) {
	va_list arg;
	unsigned int* value;
	
	va_start(arg, fd);
	value = va_arg(arg, unsigned int*);
	va_end(arg);
	
	return ioctl(fd, _IOW('A', 'r', int), *value);
}

static inline int get_rate(int fd, ...) {
	va_list arg;
	unsigned int* value;
	int ret;
	
	va_start(arg, fd);
	value = va_arg(arg, unsigned int*);
	va_end(arg);
	
	ret = ioctl(fd, _IOR('A', 'r', int), value);
	printf("rate:\t%08x\n", *value);
	
	return ret;
}

static inline int set_scale(int fd, ...) {
	va_list arg;
	unsigned int* value;
	
	va_start(arg, fd);
	value = va_arg(arg, unsigned int*);
	va_end(arg);
	
	return ioctl(fd, _IOW('A', 'c', int), *value);
}

static inline int get_scale(int fd, ...) {
	va_list arg;
	unsigned int* value;
	int ret;
	
	va_start(arg, fd);
	value = va_arg(arg, unsigned int*);
	va_end(arg);
	
	ret = ioctl(fd, _IOR('A', 'c', int), value);
	printf("scale:\t%08x\n", *value);
	
	return ret;
}

static inline int set_samples(int fd, ...) {
	va_list arg;
	unsigned int* value;
	
	va_start(arg, fd);
	value = va_arg(arg, unsigned int*);
	va_end(arg);
	
	return ioctl(fd, _IOW('A', 'p', int), *value);
}

static inline int get_samples(int fd, ...) {
	va_list arg;
	unsigned int* value;
	int ret;
	
	va_start(arg, fd);
	value = va_arg(arg, unsigned int*);
	va_end(arg);
	
	ret = ioctl(fd, _IOR('A', 'p', int), value);
	printf("samples:\t%08x\n", *value);
	
	return ret;
}

static inline int reset_core(int fd, ...) {
	return ioctl(fd, _IO('A', 's'));
}

static inline int disable_core(int fd, ...) {
	return ioctl(fd, _IO('A', 'd'));
}

static inline int enable_core(int fd, ...) {
	return ioctl(fd, _IO('A', 'e'));
}

int main(int argc, char** argv) {
#	define MAKE_ENTRY(x) { .name = #x, .fn = x }
	const struct {
			const char* name;
			const fn_ptr fn;
		} progs[] = {
				MAKE_ENTRY(set_rate),
				MAKE_ENTRY(get_rate),
				MAKE_ENTRY(set_scale),
				MAKE_ENTRY(get_scale),
				MAKE_ENTRY(set_samples),
				MAKE_ENTRY(get_samples),
				MAKE_ENTRY(reset_core),
				MAKE_ENTRY(disable_core),
				MAKE_ENTRY(enable_core)
			};
#	undef MAKE_ENTRY
	char* chardev = NULL;
	char* prog_name;
	int opt, fd, ret = EXIT_FAILURE;
	unsigned int ioctl_arg = 0;


	static const struct option long_opts[] = {
			{ "device",		required_argument,	NULL,	'f' },
			{ "value",		required_argument,	NULL,	'v' },
			{ 0, 0, NULL, 0 }
		};

	while((opt = getopt_long(argc, argv, "f:", long_opts, (int*)NULL)) >= 0) {
		switch(opt) {
			case 'f':
				free(chardev);
				chardev = strdup(optarg);
				break;
			case 'v':
				ioctl_arg = strtol(optarg, (char**)NULL, 16);
				break;
			default:
				break;
		}
	}

	if(!(prog_name = rindex(argv[0], '/'))) {
		prog_name = argv[0];
	} else {
		prog_name++;
	}

	if(!chardev) {
		chardev = DEFAULT_CHARDEV;
	}

	if((fd = open(chardev, O_RDWR)) < 1) {
		perror("open()");
		goto err_out;
	}

	for(size_t i = 0; i < sizeof(progs)/sizeof(progs[0]); i++) {
		if(PGM_LAUNCH(progs[i], ioctl_arg)) {
			perror("ioctl()");
			goto err_ioctl;
		};
	}

	ret = EXIT_SUCCESS;

err_ioctl:
	close(fd);
err_out:
	return ret;
}
