BOT=TURRET

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

turret_control_srcs=$(shell find src/turret_control/*.c)
turret_control_objs=$(turret_control_srcs:.c=.o)

sensors_control_srcs=$(shell find src/sensors_control/*.c)
sensors_control_objs=$(sensors_control_srcs:.c=.o)

sensors_speed_test_srcs=$(shell find src/sensors_speed_test/*.c)
sensors_speed_test_objs=$(sensors_speed_test_srcs:.c=.o)

lcm_rpmsg_bridge_srcs=$(shell find src/lcm_rpmsg_bridge/*.c)
lcm_rpmsg_bridge_objs=$(lcm_rpmsg_bridge_srcs:.c=.o)

sbus_radio_srcs=$(shell find $(COMMON_MODULES_DIR)/src/sbus_radio/*.c)
sbus_radio_objs=$(sbus_radio_srcs:.c=.o)

#telemetry_srcs=$(shell find $(COMMON_MODULES_DIR)/src/telemetry/*.c)

telemetry_srcs= $(COMMON_MODULES_DIR)/src/telemetry/rfd900x.c $(COMMON_MODULES_DIR)/src/telemetry/telemetry.c $(COMMON_MODULES_DIR)/src/telemetry/main.c
telemetry_objs=$(telemetry_srcs:.c=.o)

telem_test_srcs= $(COMMON_MODULES_DIR)/src/telemetry/rfd900x.c $(COMMON_MODULES_DIR)/src/telemetry/telemetry.c $(COMMON_MODULES_DIR)/src/telemetry/test_telem.c
telem_test_objs=$(telem_test_srcs:.c=.o)

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

turret_control: CFLAGS+= $(TOMLC99_CFLAGS)

fake_sbus_radio: CFLAGS+=$(NCURSES_CFLAGS)
fake_sbus_radio: LDFLAGS+=$(NCURSES_LDFLAGS)

all: pru_targets libs targets

targets: turret_control sensors_control lcm_rpmsg_bridge sbus_radio telemetry

pru_targets:
	@$(MAKE) -C "src/hammer_control"

libs: $(TOMLC99_STATICLIB) $(SCLOG4C_STATICLIB) $(MESSAGELIB)

clean:
	rm -f $(turret_control_objs)
	rm -f $(sensors_control_objs)
	rm -f $(lcm_rpmsg_bridge_objs)
	rm -f turret_control sensors_control lcm_rpmsg_bridge sbus_radio telemetry fake_sbus_radio test_telem
	rm -f $(MESSAGELIB)
	rm -f $(sbus_radio_objs)
	rm -f $(COMMON_MODULES_DIR)/src/sbus_radio/fake/fake_radio.o
	rm -f $(telemetry_objs)
	rm -f $(telem_test_objs) 
	rm -fr inc/lcm src/lcm
	rm -fr $(COMMON_MODULES_DIR)/inc/lcm $(COMMON_MODULES_DIR)/src/lcm
	$(MAKE) -C src/hammer_control clean
	$(MAKE) -C $(TOML_DIR) clean
	$(MAKE) -C $(SCLOG4C_DIR) clean

turret_control: $(turret_control_objs) $(MESSAGELIB) $(SCLOG4C_STATICLIB) $(TOMLC99_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

turret_control_cap: turret_control
	sudo setcap "cap_sys_nice=ep" ./turret_control

sensors_control: $(sensors_control_objs) $(MESSAGELIB) $(SCLOG4C_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

sensors_control_cap: sensors_control
	sudo setcap "cap_sys_nice=ep" ./sensors_control

sensors_speed_test: $(sensors_speed_test_objs) 
	gcc $^ -o $@ $(LDFLAGS)

sensors_speed_test_cap: sensors_speed_test
	sudo setcap "cap_sys_nice=ep" ./sensors_speed_test

lcm_rpmsg_bridge: $(lcm_rpmsg_bridge_objs) $(MESSAGELIB) $(SCLOG4C_STATICLIB)
	gcc $^ -o $@ $(LDFLAGS)

lcm_rpmsg_bridge_cap: lcm_rpmsg_bridge
	sudo setcap "cap_sys_nice=ep" ./lcm_rpmsg_bridge

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

$(TOML_DIR)/libtoml.a: $(TOML_DIR)/toml.c $(TOML_DIR)/toml.h
	$(MAKE) -C $(TOML_DIR)

$(SCLOG4C_DIR)/src/libsclog4c.a: $(SCLOG4C_DIR)/src/sclog4c.c
	$(MAKE) -C $(SCLOG4C_DIR)/src

%.pdf: %.dot
	dot -Tpdf $< -o $@

.PHONY: all clean libs targets
