INCLUDES := ../../kernel/ko

CFLAGS := -O2 -Wall -Werror \
			-ffunction-sections -fdata-sections \
			$(addprefix -I,$(INCLUDES))

LDFLAGS := -O2 -Wl,--gc-sections

SYMLINKS := set_rate

CC := gcc
LD := gcc

SRC1 := set_rate.c
OBJ1 := $(SRC1:%.c=%.o)

#SRC2 := set_value.c
#OBJ2 := set_value.o

ELF1 := set_rate
#ELF2 := set_value

.PHONY: clean all

all: $(ELF1) $(SYMLINKS) $(ELF2)

clean:
	-rm -f $(ELF1) $(OBJ1) $(ELF2) $(OBJ2) $(SYMLINKS)

$(ELF1): $(OBJ1)
	$(LD) $(LDFLAGS) -o $@ $<

#$(ELF2): $(OBJ2)
#	$(LD) $(LDFLAGS) -o $@ $<

$(SYMLINKS): $(ELF1)
	ln -s $< $@

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<
