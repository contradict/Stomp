TEMPLATEROOT=.

# compilation flags for gdb

CFLAGS  = -O1 -g
# CFLAGS  = -O3
ASFLAGS = -g 

# object files

OBJS=$(STARTUP) src/main.o src/feedback_adc.o src/linearize_feedback.o src/storage.o
OBJS += src/f722-nucleo-blinky.o

CFLAGS += -I inc

# include common make file

include $(TEMPLATEROOT)/Makefile.common
