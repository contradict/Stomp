
CFLAGS=-g3

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
NCURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)
NCURSES_LDFLAGS=$(shell pkg-config --libs ncurses)

all: modbus rs485test enfield ui

modbus_%.o: modbus_%.c
	gcc -c $(CFLAGS) $(NCURSES_CFLAGS) $(MODBUS_CFLAGS) $? -o $@

modbus: modbus_test.o modbus_device.o
	gcc $(LDFLAGS) $(MODBUS_LDFLAGS) $^ -o $@

enfield: modbus_enfield.o modbus_device.o
	gcc $(LDFLAGS) $(MODBUS_LDFLAGS) $^ -o $@

ui: modbus_ui.o modbus_device.o
	gcc $(LDFLAGS) $(MODBUS_LDFLAGS) $(NCURSES_LDFLAGS) $^ -o $@

rs485test: modbus_crc.o

clean:
	rm *.o modbus rs485test
