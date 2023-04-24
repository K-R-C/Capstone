DTSO := $(wildcard *.dtso)
DTBO := $(DTSO:%.dtso=%.dtbo)


.PHONY: all clean

all: $(DTBO)

clean:
	-rm -f $(DTBO)

%_prog: %.dtbo
	mkdir -p /sys/kernel/config/device-tree/overlays/${@:%_prog=%} && \
		cat $< > /sys/kernel/config/device-tree/overlays/${@:%_prog=%}/dtbo
%_remove: %.dtbo
	rmdir /sys/kernel/config/device-tree/overlays/${@:%_remove=%}

%_install: %.dtbo
	cp $< /boot/dtbos/$<

%.dtbo: %.dtso
	dtc -@ -O dtb -I dts -o $@ $<
