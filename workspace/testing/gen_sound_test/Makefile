ELF := load_bram

SRC := load_bram.c

OBJ := $(SRC:%.c=%.o)
OBJ += binary_data.o

CC := gcc
LD := gcc
OBJCOPY := objcopy

CFLAGS := -O2

.PHONY: all clean

all: $(ELF)

clean:
	-rm -f $(ELF) $(OBJ) binary_file.bin

$(ELF): $(OBJ)
	$(LD) -o $(ELF) $(OBJ)

binary_data.o: binary_file.bin
	$(OBJCOPY) -I binary -O elf32-littlearm \
		--rename-section .data=.rodata,contents,alloc,load,readonly,data  \
		--redefine-sym _binary_binary_file_bin_end=binary_data_end \
		--redefine-sym _binary_binary_file_bin_size=binary_data_size \
		--redefine-sym _binary_binary_file_bin_start=binary_data_start \
		$< $@

binary_file.bin: gen_mem_data.py make_binary.py
	./gen_mem_data.py | ./make_binary.py --output-file $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
