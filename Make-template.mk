# Template for a makefile to compile using BoostC.

include ../src/reuse/Make-boost.mk

# Rules for a PIC project with this directory structure.

# Set this to all the places source or object files could be found.
VPATH = ../src ../src/reuse
BINDIR = ../bin

.PHONY: all binary dist clean

all: binary

.PHONY: dist
dist: binary
	# Promote the compiled binary to the /bin directory.
	# Usually only done for Release builds.
	cp ${BINARY} ${BINDIR}

.PHONY: clean
clean:
	rm -f *.o *.asm *.lst *.map *.casm *.hex *.cof *.stat *.tree *.pp.dep .depend


# Defines for this project.

CHIP = PIC18F45K22

CFLAGS = -t ${CHIP} -I "../src;../src/reuse" -O2 -i -d REF_BUG
LDFLAGS = -t ${CHIP} -ld "C:\Program Files (x86)\SourceBoost\lib" -beep
LDLIBS = libc.pic18.lib

BINARY = MyProject.hex

# All the modules required for the main binary.
SRCS = MyProject.c OtherModule.c

include ../src/reuse/Make-binary.mk
