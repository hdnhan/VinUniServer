# C++ compiler flag
CCFLAGS := -Wall -DRELEASE -O3 -s
# Linker library flags
LDLIBS   := -lnvidia-ml

all:
	$(CC) $(CCFLAGS) main.c $(LDLIBS) -o smi
clean:
	@$(RM) -rv smi
