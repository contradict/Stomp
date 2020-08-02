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

CFLAGS=-g3 -I inc $(LCM_CFLAGS)
LDFLAGS=-lm $(LCM_LDFLAGS)

src/leg_control/%.o: CFLAGS+= -I ../Common $(MODBUS_CFLAGS) $(RINGBUF_CFLAGS)

leg_control: LDFLAGS+= $(MODBUS_LDFLAGS) $(RINGBUF_STATICLIB)

all: leg_control sbus_radio

clean:
	rm -r ringbuf/lib ringbuf/include
	rm -r inc/lcm src/lcm
	rm leg_control sbus_radio

leg_control: $(leg_control_objs) ../Common/modbus_device.o src/lcm/messages.a $(RINGBUF_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

sbus_radio: src/sbus_radio/main.o src/lcm/messages.a
	gcc $^ -o $@ $(LDFLAGS)
	
src/lcm/messages.a: $(message_objs)
	ar rcs $@ $^

src/lcm/stomp_%.c: lcm/%.lcm
	mkdir -p src/lcm
	mkdir -p inc/lcm
	lcm-gen -c --c-cpath src/lcm/ --c-hpath inc/lcm/ --cinclude lcm $<

inc/lcm/stomp_%.h: lcm/%.lcm
	mkdir -p src/lcm
	mkdir -p inc/lcm
	lcm-gen -c --c-cpath src/lcm/ --c-hpath inc/lcm/ --cinclude lcm $<

ringbuf/lib/libringbuf.a: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C ringbuf/src LIBDIR=`pwd`/ringbuf/lib INCDIR=`pwd`/ringbuf/include install

ringbuf/include/ringbuf.h: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C ringbuf/src LIBDIR=`pwd`/ringbuf/lib INCDIR=`pwd`/ringbuf/include install


%.o: %.c
	gcc -c $(CFLAGS) $? -o $@

.PHONY: all messages ringbuf clean
