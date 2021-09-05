TEMPLATEROOT=.

# compilation flags for gdb

OPT_FLAGS = -O0
CFLAGS  = -g3
# CFLAGS  = -O3
ASFLAGS = -g3

# object files

OBJS=$(STARTUP)  src/main.o
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

Kinematics.pdf: Kinematics.md LiftCurlGeometryCplx.jpg SwingGeometryCplx.jpg LegDrawing.png
	pandoc Kinematics.md -o $@

src/kinematics.o: src/kinematics.c inc/kinematics.h inc/linearize_feedback.h inc/kinematics.h inc/modbus.h inc/enfield.h
	$(CC) -c $(CFLAGS) -O3 $< -o $@
	$(CC) -MM $(CFLAGS) -O3 $< > $*.d

src/linearize_feedback.o: src/linearize_feedback.c inc/linearize_feedback.h inc/export/joint.h inc/feedback_adc.h inc/status_led.h inc/modbus.h inc/kinematics.h Drivers/BSP/Components/ads57x4/ads57x4.h Drivers/BSP/ChompLegBoard/chomplegboard.h
	$(CC) -c $(CFLAGS) -O3 $< -o $@
	$(CC) -MM $(CFLAGS) -O3 $< > $*.d
