
CFLAGS=-g3

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)

all: modbus rs485test enfield

modbus_%.o: modbus_%.c
	gcc -c $(CFLAGS) $(MODBUS_CFLAGS) $? -o $@

modbus: modbus_test.o modbus_device.o
	gcc $(LDFLAGS) $(MODBUS_LDFLAGS) $^ -o $@

enfield: modbus_enfield.o modbus_device.o
	gcc $(LDFLAGS) $(MODBUS_LDFLAGS) $^ -o $@

rs485test: modbus_crc.o

clean:
	rm *.o modbus rs485test
