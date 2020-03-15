
CFLAGS=-g3 -I ../../LegBoard/Firmware/inc/export

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
NCURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
ZMQ_CFLAGS=$(shell pkg-config --cflags libzmq)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)
NCURSES_LDFLAGS=$(shell pkg-config --libs ncurses)
ZMQ_LDFLAGS=$(shell pkg-config --libs libzmq)

all: modbus rs485test enfield ui test_publisher

modbus_%.o: modbus_%.c
	gcc -c $(CFLAGS) $(NCURSES_CFLAGS) $(MODBUS_CFLAGS) $? -o $@

modbus: modbus_test.o modbus_device.o
	gcc $(LDFLAGS) $^ -o $@ $(MODBUS_LDFLAGS)

enfield: modbus_enfield.o modbus_device.o
	gcc $(LDFLAGS) $^ -o $@ $(MODBUS_LDFLAGS)

ui: modbus_ui.o modbus_device.o
	gcc $(LDFLAGS) $^ -o $@ $(NCURSES_LDFLAGS) $(MODBUS_LDFLAGS)

rs485test: modbus_crc.o

rfd900x.o: rfd900x.c
	gcc -c $(CFLAGS) rfd900x.c

telem.o: telem.c
	gcc -c $(CFLAGS) telem.c

test_publisher.o: test_publisher.c
	gcc -c $(CFLAGS) test_publisher.c

test_publisher: test_publisher.o telem.o rfd900x.o
	gcc $(LDFLAGS) -o test_publisher test_publisher.o telem.o rfd900x.o

clean:
	rm *.o modbus rs485test
