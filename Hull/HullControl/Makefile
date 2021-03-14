BOT=HULL

COMMON_MODULES_DIR=../../Common/modules

LCM_LOCAL_DIR=lcm
LCM_COMMON_DIR=$(COMMON_MODULES_DIR)/lcm
LCM_CFLAGS=$(shell pkg-config --cflags lcm)
LCM_LDFLAGS=$(shell pkg-config --libs lcm)

message_defs_local=$(shell find $(LCM_LOCAL_DIR)/*.lcm)
message_hdrs_local=$(subst $(LCM_LOCAL_DIR)/,inc/lcm/stomp_,$(message_defs_local:.lcm=.h))
message_srcs_local=$(subst $(LCM_LOCAL_DIR)/,src/lcm/stomp_,$(message_defs_local:.lcm=.c))
message_java_local=$(subst $(LCM_LOCAL_DIR)/,java/stomp/,$(message_defs_local:.lcm=.java))

message_defs_common=$(shell find $(LCM_COMMON_DIR)/*.lcm)
message_hdrs_common=$(subst $(LCM_COMMON_DIR)/,$(COMMON_MODULES_DIR)/inc/lcm/stomp_,$(message_defs_common:.lcm=.h))
message_srcs_common=$(subst $(LCM_COMMON_DIR)/,$(COMMON_MODULES_DIR)/src/lcm/stomp_,$(message_defs_common:.lcm=.c))
message_java_common=$(subst $(LCM_COMMON_DIR)/,$(COMMON_MODULES_DIR)/java/stomp/,$(message_defs_common:.lcm=.java))

message_srcs=$(message_srcs_local) $(message_srcs_common)
message_objs=$(message_srcs:.c=.o)

message_java=$(message_java_local) $(message_java_common)
message_clss=$(message_java:.java=.class)

MESSAGELIB=src/lcm/messages.a

leg_control_srcs=$(shell find src/leg_control/*.c)
leg_control_objs=$(leg_control_srcs:.c=.o)

sbus_radio_srcs=$(shell find $(COMMON_MODULES_DIR)/src/sbus_radio/*.c)
sbus_radio_objs=$(sbus_radio_srcs:.c=.o)

#telemetry_srcs=$(shell find $(COMMON_MODULES_DIR)/src/telemetry/*.c)

telemetry_srcs= $(COMMON_MODULES_DIR)/src/telemetry/rfd900x.c $(COMMON_MODULES_DIR)/src/telemetry/telemetry.c $(COMMON_MODULES_DIR)/src/telemetry/main.c
telemetry_objs=$(telemetry_srcs:.c=.o)

telem_test_srcs= $(COMMON_MODULES_DIR)/src/telemetry/rfd900x.c $(COMMON_MODULES_DIR)/src/telemetry/telemetry.c $(COMMON_MODULES_DIR)/src/telemetry/test_telem.c
telem_test_objs=$(telem_test_srcs:.c=.o)

MODBUS_CFLAGS=$(shell pkg-config --cflags libmodbus)
MODBUS_LDFLAGS=$(shell pkg-config --libs libmodbus)

RINGBUF_DIR=../../Common/external/ringbuf
RINGBUF_CFLAGS=-I $(RINGBUF_DIR)/include
RINGBUF_STATICLIB=$(RINGBUF_DIR)/lib/libringbuf.a

TOML_DIR=../../Common/external/tomlc99
TOMLC99_CFLAGS=-I $(TOML_DIR)
TOMLC99_STATICLIB=$(TOML_DIR)/libtoml.a

SCLOG4C_DIR=../../Common/external/sclog4c
SCLOG4C_CFLAGS=-I $(SCLOG4C_DIR)/include
SCLOG4C_STATICLIB=$(SCLOG4C_DIR)/src/libsclog4c.a

NCURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
NCURSES_LDFLAGS=$(shell pkg-config --libs ncurses)

CFLAGS=-g3 -std=gnu11 -I inc -I $(COMMON_MODULES_DIR)/inc -Wall -D$(BOT) $(LCM_CFLAGS) $(SCLOG4C_CFLAGS)
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
	rm -f LegModes.pdf
	rm -f src/leg_control/*.o
	rm -f src/leg_control/test/*.o
	rm -f leg_control sbus_radio telemetry fake_sbus_radio test_telem
	rm -f $(MESSAGELIB)
	rm -f $(sbus_radio_objs)
	rm -f $(COMMON_MODULES_DIR)/src/sbus_radio/fake/fake_radio.o
	rm -f $(telemetry_objs)
	rm -f $(telem_test_objs) 
	rm -fr inc/lcm src/lcm
	rm -fr $(COMMON_MODULES_DIR)/inc/lcm $(COMMON_MODULES_DIR)/src/lcm
	$(MAKE) -C $(TOML_DIR) clean
	$(MAKE) -C $(RINGBUF_DIR)/src clean
	$(MAKE) -C $(SCLOG4C_DIR) clean

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

$(MESSAGELIB): $(message_objs)
	ar rcs $@ $^

sbus_radio: $(sbus_radio_objs) $(MESSAGELIB)
	gcc $(COMMON_MODULES_DIR)/src/sbus_radio/main.o $(MESSAGELIB) -o $@ $(LDFLAGS) $(LCM_LDFLAGS)

fake_sbus_radio: $(COMMON_MODULES_DIR)/src/sbus_radio/fake/fake_radio.o $(MESSAGELIB)
	gcc $^ -o $@ $(LDFLAGS) $(LCM_LDFLAGS)

telemetry: $(telemetry_objs) $(MESSAGELIB) 
	gcc $^ -o $@ $(LDFLAGS) $(LCM_LDFLAGS)

test_telem: $(telem_test_objs) $(MESSAGELIB) $(SCLOG4C_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

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

$(COMMON_MODULES_DIR)/src/lcm/stomp_%.c: $(LCM_COMMON_DIR)/%.lcm | $(COMMON_MODULES_DIR)/src/lcm $(COMMON_MODULES_DIR)/inc/lcm
	lcm-gen -c --c-cpath $(COMMON_MODULES_DIR)/src/lcm/ --c-hpath $(COMMON_MODULES_DIR)/inc/lcm/ --cinclude lcm $<

$(COMMON_MODULES_DIR)/inc/lcm/stomp_%.h: $(LCM_COMMON_DIR)/%.lcm | $(COMMON_MODULES_DIR)/src/lcm $(COMMON_MODULES_DIR)/inc/lcm
	lcm-gen -c --c-cpath $(COMMON_MODULES_DIR)/src/lcm/ --c-hpath $(COMMON_MODULES_DIR)/inc/lcm/ --cinclude lcm $<

$(COMMON_MODULES_DIR)/java/stomp/%.java: $(LCM_COMMON_DIR)/%.lcm
	lcm-gen -j --jpath java $<

$(COMMON_MODULES_DIR)/src/lcm:
	mkdir -p $(COMMON_MODULES_DIR)/src/lcm

$(COMMON_MODULES_DIR)/inc/lcm:
	mkdir -p $(COMMON_MODULES_DIR)/inc/lcm

stomp.jar: $(message_clss)
	jar -cf stomp.jar -C java stomp/control_radio.class -C java stomp/modbus.class -C java stomp/telemetry_leg.class

%.class: %.java
	javac -cp /usr/share/java/lcm.jar $<

$(RINGBUF_DIR)/lib/libringbuf.a: $(RINGBUF_DIR)/src/ringbuf.c $(RINGBUF_DIR)/src/ringbuf.h
	$(MAKE) -C $(RINGBUF_DIR)/src LIBDIR=`pwd`/$(RINGBUF_DIR)/lib INCDIR=`pwd`/$(RINGBUF_DIR)/include install

$(RINGBUF_DIR)/include/ringbuf.h: $(RINGBUF_DIR)/src/ringbuf.c $(RINGBUF_DIR)/src/ringbuf.h
	$(MAKE) -C $(RINGBUF_DIR)/src LIBDIR=`pwd`/$(RINGBUF_DIR)/lib INCDIR=`pwd`/$(RINGBUF_DIR)/include install

$(TOML_DIR)/libtoml.a: $(TOML_DIR)/toml.c $(TOML_DIR)/toml.h
	$(MAKE) -C $(TOML_DIR)

$(SCLOG4C_DIR)/src/libsclog4c.a: $(SCLOG4C_DIR)/src/sclog4c.c
	$(MAKE) -C $(SCLOG4C_DIR)/src

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
