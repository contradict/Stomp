message_defs=$(shell find lcm/*.lcm)
message_hdrs=$(subst lcm/,inc/lcm/stomp_,$(message_defs:.lcm=.h))
message_srcs=$(subst lcm/,src/lcm/stomp_,$(message_defs:.lcm=.c))
message_objs=$(message_srcs:.c=.o)
message_java=$(subst lcm/,java/stomp/,$(message_defs:.lcm=.java))
message_clss=$(message_java:.java=.class)
MESSAGELIB=src/lcm/messages.a

leg_control_srcs=$(shell find src/leg_control/*.c)
leg_control_objs=$(leg_control_srcs:.c=.o)

sbus_radio_srcs=$(shell find src/sbus_radio/*.c)
sbus_radio_objs=$(sbus_radio_srcs:.c=.o)

telemetry_srcs=$(shell find src/telemetry/*.c)
telemetry_objs=$(telemetry_srcs:.c=.o)

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)

LCM_CFLAGS=$(shell pkg-config --cflags lcm)
LCM_LDFLAGS=$(shell pkg-config --libs lcm)

RINGBUF_CFLAGS=-I ringbuf/include
RINGBUF_STATICLIB=ringbuf/lib/libringbuf.a

TOMLC99_CFLAGS=-I tomlc99
TOMLC99_STATICLIB=tomlc99/libtoml.a

SCLOG4C_CFLAGS=-I sclog4c/include
SCLOG4C_STATICLIB=sclog4c/src/libsclog4c.a

NCURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
NCURSES_LDFLAGS=$(shell pkg-config --libs ncurses)

CFLAGS=-g3 -std=gnu11 -I inc -Wall $(LCM_CFLAGS) $(SCLOG4C_CFLAGS)
LDFLAGS=-lm $(LCM_LDFLAGS) $(SCLOG4C_STATICLIB)

src/leg_control/%.o: CFLAGS+= -I ../Common $(MODBUS_CFLAGS) $(RINGBUF_CFLAGS)


leg_control: CFLAGS+= -I ../Common -I ../../LegBoard/Firmware/inc/export/ $(MODBUS_CFLAGS) $(RINGBUF_CFLAGS) $(TOMLC99_CFLAGS)
leg_control: LDFLAGS+= $(MODBUS_LDFLAGS)
test_gain_set: CFLAGS+= -I ../Common -I ../../LegBoard/Firmware/inc/export/ $(MODBUS_CFLAGS)
test_gain_set: LDFLAGS+= $(MODBUS_LDFLAGS)

fake_sbus_radio: CFLAGS+=$(NCURSES_CFLAGS)
fake_sbus_radio: LDFLAGS+=$(NCURSES_LDFLAGS)

all: libs targets

targets: leg_control sbus_radio telemetry

LegModes.pdf: LegModes.dot

libs: $(RINGBUF_STATICLIB) $(TOMLC99_STATICLIB) $(SCLOG4C_STATICLIB) $(MESSAGELIB)

clean:
	rm -f src/leg_control/*.o
	rm -f src/leg_control/test/*.o
	rm -f src/sbus_radio/*.o
	rm -f src/sbus_radio/fake/*.o
	rm -fr ringbuf/lib ringbuf/include
	rm -fr inc/lcm src/lcm
	rm -f leg_control sbus_radio
	rm -f LegModes.pdf
	$(MAKE) -C tomlc99 clean
	$(MAKE) -C ringbuf/src clean
	$(MAKE) -C sclog4c clean

leg_control: $(leg_control_objs) ../Common/modbus_device.o $(MESSAGELIB) $(RINGBUF_STATICLIB) $(TOMLC99_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

leg_control_cap: leg_control
	sudo setcap "cap_sys_nice=ep" ./leg_control

lissajou: CFLAGS+= -I ../../LegBoard/Firmware/inc/export
lissajou: LDFLAGS+= $(MODBUS_LDFLAGS)

lissajou: src/leg_control/test/lissajou.o src/leg_control/modbus_utils.o src/leg_control/rate_timer.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS)

lissajou_cap: lissajou
	sudo setcap "cap_sys_nice=ep" ./lissajou

test_gain_set: src/leg_control/test/test_gain_set.o src/leg_control/modbus_utils.o ../Common/modbus_device.o
	gcc $^ -o $@ $(LDFLAGS)

sbus_radio: $(sbus_radio_objs) $(MESSAGELIB)
	gcc src/sbus_radio/main.o $(MESSAGELIB) -o $@ $(LDFLAGS) $(LCM_LDFLAGS)

fake_sbus_radio: src/sbus_radio/fake/fake_radio.o $(MESSAGELIB)
	gcc $^ -o $@ $(LDFLAGS) $(LCM_LDFLAGS)

telemetry: $(telemetry_objs) $(MESSAGELIB)
	gcc $^ -o $@ $(LDFLAGS) $(LCM_LDFLAGS)

$(MESSAGELIB): $(message_objs)
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

java/stomp/%.java: lcm/%.lcm
	lcm-gen -j --jpath java $<

stomp.jar: $(message_clss)
	jar -cf stomp.jar -C java stomp/control_radio.class -C java stomp/modbus.class -C java stomp/telemetry_leg.class

%.class: %.java
	javac -cp /usr/share/java/lcm.jar $<

ringbuf/lib/libringbuf.a: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C ringbuf/src LIBDIR=`pwd`/ringbuf/lib INCDIR=`pwd`/ringbuf/include install

ringbuf/include/ringbuf.h: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C ringbuf/src LIBDIR=`pwd`/ringbuf/lib INCDIR=`pwd`/ringbuf/include install

tomlc99/libtoml.a: tomlc99/toml.c tomlc99/toml.h
	$(MAKE) -C tomlc99

sclog4c/src/libsclog4c.a: sclog4c/src/sclog4c.c
	$(MAKE) -C sclog4c/src

%.pdf: %.dot
	dot -Tpdf $< -o $@

lcmspy: stomp.jar
	CLASSPATH=`pwd`/stomp.jar lcm-spy

imu_srcs=src/imu/spidevlib.c src/imu/imu.c
imu_hdrs=src/imu/spidevlib.h
imu_objs=$(imu_srcs:.c=.o)

imu: $(imu_objs) $(imu_hdrs)
	$(CC) $(imu_objs) -o $@

.PHONY: all clean libs targets lcmspy
