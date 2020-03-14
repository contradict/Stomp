TEMPLATEROOT=.

# compilation flags for gdb

CFLAGS  = -O0 -g3
# CFLAGS  = -O3
ASFLAGS = -g3

# object files

OBJS=$(STARTUP) src/main.o
OBJS += src/stm32f7xx_hal_timebase_tim.o
OBJS += src/storage.o
OBJS += src/status_led.o
OBJS += src/debug_pin.o
OBJS += src/feedback_adc.o src/linearize_feedback.o
OBJS += src/uart.o
OBJS += src/modbus.o src/modbus_coils.o src/modbus_discrete_inputs.o
OBJS += src/modbus_holding_registers.o src/modbus_common.o src/modbus_input_registers.o
OBJS += src/enfield.o
OBJS += src/kinematics.o
# OBJS += src/f722-nucleo-blinky.o

CFLAGS += -I inc

# include common make file

include $(TEMPLATEROOT)/Makefile.common

# Linearization documentation

Kinematics.pdf: Kinematics.md LiftCurlGeometry.jpg
	pandoc Kinematics.md -o $@


