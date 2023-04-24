obj-m += pwm_driver.o
capstone_pwm_driver-objs := pwm_driver.o

all:
	$(MAKE) -C /usr/lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	$(MAKE) -C /usr/lib/modules/$(shell uname -r)/build M=$(PWD) clean
