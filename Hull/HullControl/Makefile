message_defs=$(shell find lcm/*.lcm)
message_hdrs=$(subst lcm/,inc/lcm/stomp_,$(message_defs:.lcm=.h))
message_srcs=$(subst lcm/,src/lcm/stomp_,$(message_defs:.lcm=.c))
message_objs=$(message_srcs:.c=.o)

leg_control_srcs=$(shell find src/leg_control/*.c)
leg_control_objs=$(leg_control_srcs:.c=.o)

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)

LCM_CFLAGS=$(shell pkg-config --cflags lcm)
LCM_LDFLAGS=$(shell pkg-config --libs lcm)

RINGBUF_CFLAGS=-I ringbuf/include
RINGBUF_STATICLIB=ringbuf/lib/libringbuf.a

TOMLC99_CFLAGS=-I tomlc99
TOMLC99_STATICLIB=tomlc99/libtoml.a

CFLAGS=-g3 -I inc $(LCM_CFLAGS)
LDFLAGS=-lm $(LCM_LDFLAGS)

src/leg_control/%.o: CFLAGS+= -I ../Common $(MODBUS_CFLAGS) $(RINGBUF_CFLAGS)

leg_control: CFLAGS+= -I ../Common -I ../../LegBoard/Firmware/inc/export/ $(MODBUS_CFLAGS) $(RINGBUF_CFLAGS) $(TOMLC99_CFLAGS)
leg_control: LDFLAGS+= $(MODBUS_LDFLAGS)

all: leg_control sbus_radio

clean:
	rm -fr ringbuf/lib ringbuf/include
	rm -fr inc/lcm src/lcm
	rm -f leg_control sbus_radio
	$(MAKE) -C tomlc99 clean
	$(MAKE) -C ringbuf/src clean

leg_control: $(leg_control_objs) ../Common/modbus_device.o src/lcm/messages.a $(RINGBUF_STATICLIB) $(TOMLC99_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

sbus_radio: src/sbus_radio/main.o src/lcm/messages.a
	gcc $^ -o $@ $(LDFLAGS)

src/lcm/messages.a: $(message_objs)
	ar rcs $@ $^

.SECONDARY: $(message_srcs)

src/lcm/stomp_%.c: lcm/%.lcm | src/lcm inc/lcm
	lcm-gen -c --c-cpath src/lcm/ --c-hpath inc/lcm/ --cinclude lcm $<

inc/lcm/stomp_%.h: lcm/%.lcm | src/lcm inc/lcm
	lcm-gen -c --c-cpath src/lcm/ --c-hpath inc/lcm/ --cinclude lcm $<

src/lcm:
	mkdir -p src/lcm

inc/lcm:
	mkdir -p inc/lcm

ringbuf/lib/libringbuf.a: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C ringbuf/src LIBDIR=`pwd`/ringbuf/lib INCDIR=`pwd`/ringbuf/include install

ringbuf/include/ringbuf.h: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C ringbuf/src LIBDIR=`pwd`/ringbuf/lib INCDIR=`pwd`/ringbuf/include install

tomlc99/libtoml.a: tomlc99/toml.c tomlc99/toml.h
	$(MAKE) -C tomlc99

%.o: %.c
	gcc -c $(CFLAGS) $? -o $@

.PHONY: all clean
