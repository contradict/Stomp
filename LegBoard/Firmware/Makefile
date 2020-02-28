TEMPLATEROOT=.

# compilation flags for gdb

CFLAGS  = -O0 -g
# CFLAGS  = -O3
ASFLAGS = -g 

# object files

OBJS=$(STARTUP) src/main.o
OBJS += src/feedback_adc.o src/linearize_feedback.o
OBJS += src/storage.o
OBJS += src/uart.o
OBJS += src/modbus.o src/modbus_coils.o src/modbus_discrete_inputs.o
OBJS += src/servo.o
OBJS += src/status_led.o
OBJS += src/stm32f7xx_hal_timebase_tim.o
# OBJS += src/f722-nucleo-blinky.o

CFLAGS += -I inc

# include common make file

include $(TEMPLATEROOT)/Makefile.common
