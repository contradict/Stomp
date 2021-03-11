message_defs=$(shell find lcm/*.lcm)
message_hdrs=$(subst lcm/,inc/lcm/stomp_,$(message_defs:.lcm=.h))
message_srcs=$(subst lcm/,src/lcm/stomp_,$(message_defs:.lcm=.c))
message_objs=$(message_srcs:.c=.o)
message_java=$(subst lcm/,java/stomp/,$(message_defs:.lcm=.java))
message_clss=$(message_java:.java=.class)
MESSAGELIB=src/lcm/messages.a

sbus_radio_srcs=$(shell find src/sbus_radio/*.c)
sbus_radio_objs=$(sbus_radio_srcs:.c=.o)

telemetry_srcs=$(shell find src/telemetry/*.c)
telemetry_objs=$(telemetry_srcs:.c=.o)

LCM_CFLAGS=$(shell pkg-config --cflags lcm)
LCM_LDFLAGS=$(shell pkg-config --libs lcm)

RINGBUF_DIR=../extern/ringbuf
RINGBUF_CFLAGS=-I $(RINGBUF_DIR)/include
RINGBUF_STATICLIB=$(RINGBUF_DIR)/lib/libringbuf.a

TOML_DIR=../extern/tomlc99
TOMLC99_CFLAGS=-I $(TOML_DIR)
TOMLC99_STATICLIB=$(TOML_DIR)/libtoml.a

SCLOG4C_DIR=../sclog4c
SCLOG4C_CFLAGS=-I $(SCLOG4C)/include
SCLOG4C_STATICLIB=$(SCLOG4C)/src/libsclog4c.a

NCURSES_CFLAGS=$(shell pkg-config --cflags ncurses)
NCURSES_LDFLAGS=$(shell pkg-config --libs ncurses)

CFLAGS=-g3 -std=gnu11 -I inc -Wall $(LCM_CFLAGS) $(SCLOG4C_CFLAGS)
LDFLAGS=-lm $(LCM_LDFLAGS) $(SCLOG4C_STATICLIB)

fake_sbus_radio: CFLAGS+=$(NCURSES_CFLAGS)
fake_sbus_radio: LDFLAGS+=$(NCURSES_LDFLAGS)

all: libs targets

targets: sbus_radio telemetry

libs: $(RINGBUF_STATICLIB) $(TOMLC99_STATICLIB) $(SCLOG4C_STATICLIB) $(MESSAGELIB)

clean:
	rm -f src/sbus_radio/*.o
	rm -f src/sbus_radio/fake/*.o
	rm -fr ringbuf/lib ringbuf/include
	rm -fr inc/lcm src/lcm
	$(MAKE) -C tomlc99 clean
	$(MAKE) -C ringbuf/src clean
	$(MAKE) -C sclog4c clean

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

$(RINGBUF_DIR)/lib/libringbuf.a: $(RINGBUF_DIR)/src/ringbuf.c $(RINGBUF_DIR)/src/ringbuf.h
	$(MAKE) -C $(RINGBUF_DIR)/src LIBDIR=`pwd`/$(RINGBUF_DIR)/lib INCDIR=`pwd`/$(RINGBUF_DIR)/include install

$(RINGBUF_DIR)/include/ringbuf.h: ringbuf/src/ringbuf.c ringbuf/src/ringbuf.h
	$(MAKE) -C $(RINGBUF_DIR)/src LIBDIR=`pwd`/$(RINGBUF_DIR)/lib INCDIR=`pwd`/$(RINGBUF_DIR)/include install

$(TOML_DIR)/libtoml.a: $(TOML_DIR)/toml.c $(TOML_DIR)/toml.h
	$(MAKE) -C tomlc99

$(SCLOG4C_DIR)/src/libsclog4c.a: $(SCLOG4C_DIR)/src/sclog4c.c
	$(MAKE) -C sclog4c/src

%.pdf: %.dot
	dot -Tpdf $< -o $@

lcmspy: stomp.jar
	CLASSPATH=`pwd`/stomp.jar lcm-spy

.PHONY: all clean libs targets lcmspy
