
CFLAGS=-g3 -I ../../LegBoard/Firmware/inc/export -I ../Common
LDFLAGS=-lm

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)

all: simplegait

modbus_%.o: modbus_%.c
	gcc -c $(CFLAGS) $(MODBUS_CFLAGS) $? -o $@

%.o: %.c
	gcc -c $(CFLAGS) $(MODBUS_CFLAGS) $? -o $@

simplegait: simple_gait.o simple_main.o realtimer.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS) $(MODBUS_LDFLAGS)

timertest: timertest.o realtimer.o
	gcc $^ -o $@ $(LDFLAGS)

.PHONY: capabilities
capabilities: simplegait
	sudo setcap "cap_sys_nice=ep" $<

clean:
	rm *.o simplegait
