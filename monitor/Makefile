# C compiler flag
CCFLAGS := -Wall -DRELEASE -O3 -s
# Linker library flags
LDLIBS  := -lnvidia-ml

# Just label for some commands, not target file
.PHONY: all clean

all:
	$(CC) $(CCFLAGS) main.c $(LDLIBS) -o smi
clean:
	@$(RM) -rv smi
