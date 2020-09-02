
CFLAGS=-g3 -I ../../LegBoard/Firmware/inc/export -I ../Common -I /usr/local/include/modbus/
LDFLAGS=-lm

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
NCURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)
NCURSES_LDFLAGS=$(shell pkg-config --libs ncurses)

all: modbus rs485test enfield ui calibration

modbus_%.o: modbus_%.c
	gcc -c $(CFLAGS) $(NCURSES_CFLAGS) $(MODBUS_CFLAGS) $? -o $@

modbus: modbus_test.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS) $(MODBUS_LDFLAGS)

enfield: modbus_enfield.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS) $(MODBUS_LDFLAGS)

ui: modbus_ui.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS) $(MODBUS_LDFLAGS) $(NCURSES_LDFLAGS)

rs485test: modbus_crc.o

calibration: CFLAGS+=-I../HullControl/tomlc99
calibration: LDFLAGS+=../HullControl/tomlc99/libtoml.a
calibration: calibration.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS) $(MODBUS_LDFLAGS)

.PHONY: capabilities
capabilities: ui
	sudo setcap "cap_sys_nice=ep" $<

clean:
	rm *.o modbus rs485test
