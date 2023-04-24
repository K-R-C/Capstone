INCLUDES := ../../kernel/ko

CFLAGS := -O2 -Wall -Werror \
			-ffunction-sections -fdata-sections \
			$(addprefix -I,$(INCLUDES))

LDFLAGS := -O2 -Wl,--gc-sections

SYMLINKS := set_rate get_rate \
			set_scale get_scale \
			set_samples get_samples \
			reset_core disable_core enable_core

CC := gcc
LD := gcc

SRC1 := ioctl_test.c
OBJ1 := $(SRC1:%.c=%.o)

ELF1 := ioctl_test

.PHONY: clean all

all: $(ELF1) $(SYMLINKS)

clean:
	-rm -f $(ELF1) $(OBJ1) $(SYMLINKS)

$(ELF1): $(OBJ1)
	$(LD) $(LDFLAGS) -o $@ $<

$(SYMLINKS): $(ELF1)
	ln -s $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
