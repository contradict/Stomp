TEMPLATEROOT=.

# compilation flags for gdb

CFLAGS  = -O1 -g
ASFLAGS = -g 

# object files

OBJS=$(STARTUP) src/main.o

# include common make file

include $(TEMPLATEROOT)/Makefile.common

.PHONY: debug
debug:
	gdb-multiarch
