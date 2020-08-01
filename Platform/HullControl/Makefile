message_defs=$(shell find lcm/*.lcm)
message_hdrs=$(subst lcm/,inc/lcm/stomp_,$(message_defs:.lcm=.h))
message_srcs=$(subst lcm/,src/lcm/stomp_,$(message_defs:.lcm=.c))
message_objs=$(message_srcs:.c=.o)

CFLAGS=-g3 -I inc -I ../Common
LDFLAGS=-lm

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)

LCM_CFLAGS=$(shell pkg-config --cflags lcm)
LCM_LDFLAGS=$(shell pkg-config --libs lcm)

all: leg_control sbus_radio

leg_control: src/leg_control/main.o ../Common/modbus_device.o src/lcm/messages.a
	gcc $^ -o $@ $(LDFLAGS) $(MODBUS_LDFLAGS) $(LCM_LDFLAGS)

sbus_radio: src/sbus_radio/main.o src/lcm/messages.a
	gcc $^ -o $@ $(LDFLAGS) $(LCM_LDFLAGS)
	
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

%.o: %.c
	gcc -c $(CFLAGS) $(MODBUS_CFLAGS) $(LCM_CFLAGS) $? -o $@

.PHONY: all messages
