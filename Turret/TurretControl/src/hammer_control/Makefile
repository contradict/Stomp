PROJ_NAME = hammer_control

CHIP = am57xx
CHIP_REV = am572x_2_0

PRU_CGT =/usr/lib/ti/pru-software-support-package

TURRET_DIR = ../..

COMMON_DIR = ../../../Common
COMMON_OUT_DIR = $(COMMON_DIR)/src/pru

COMMON_SRCS = $(COMMON_OUT_DIR)/pru_time.c $(COMMON_OUT_DIR)/pru_util.c
COMMON_OBJS = $(COMMON_SRCS:.c=.o)

LIBS =--library=$(PRU_CGT)/lib/rpmsg_lib.lib
INCLUDE = --include_path=$(PRU_CGT)/include --include_path=$(PRU_CGT)/include/$(CHIP_REV) --include_path=$(TURRET_DIR)/inc --include_path=$(COMMON_DIR)/inc/pru

STACK_SIZE = 0x100
HEAP_SIZE = 0x100
OUT_DIR = out
BIN_DIR = ../bin

LINKER_COMMAND_FILE=./$(CHIP)_pru.cmd

PRU1_0 = /sys/class/remoteproc/remoteproc4
PRU1_1 = /sys/class/remoteproc/remoteproc5
PRU2_0 = /sys/class/remoteproc/remoteproc6
PRU2_1 = /sys/class/remoteproc/remoteproc7

CFLAGS = -v3 -O2 --display_error_number --endian=little --hardware_mac=on --obj_directory=$(OUT_DIR) --pp_directory=$(OUT_DIR) -ppd -ppa
LFLAGS = --reread_libs --warn_sections --stack_size=$(STACK_SIZE) --heap_size=$(HEAP_SIZE)

TARGET = $(OUT_DIR)/$(PROJ_NAME)
FIRMWARE = am57xx-pru1_1-hammer_control

MAP = $(OUT_DIR)/$(PROJ_NAME).map
SOURCES = $(wildcard *.c)
OBJS    = $(patsubst %,$(OUT_DIR)/%,$(SOURCES:.c=.o))

all: $(TARGET)

$(TARGET): $(OBJS) $(COMMON_OBJS) $(LINKER_COMMAND_FILE)
	/usr/bin/clpru $(CFLAGS) -z -i$(PRU_CGT)/lib -i$(PRU_CGT)/include $(LFLAGS) -o $(TARGET) $(OBJS) $(COMMON_OBJS) -m$(MAP) $(LINKER_COMMAND_FILE) --library=libc.a $(LIBS)
	@echo "Installing firmware\nMust run as privileged user, using sudo"
	@sudo cp $(TARGET) /lib/firmware/$(FIRMWARE)

$(OUT_DIR)/%.o: %.c
	@mkdir -p $(OUT_DIR)
	/usr/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

$(COMMON_DIR)/src/pru/pru_time.o: $(COMMON_DIR)/src/pru/pru_time.c
	/usr/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

$(COMMON_DIR)/src/pru/pru_util.o: $(COMMON_DIR)/src/pru/pru_util.c
	/usr/bin/clpru --include_path=$(PRU_CGT)/include $(INCLUDE) $(CFLAGS) -fe $@ $<

run: $(TARGET) stop
	@echo "Installing $(TARGET) as frimware"
	@echo "Must run as privileged user, using sudo"
	@echo $(FIRMWARE) > $(PRU1_1)/firmware
	@echo "Starting $(PRU1_1)"
	@echo start > $(PRU1_1)/state


stop: 
	@PRU1_1_STATE=`cat $(PRU1_1)/state`; \
	if [ $$PRU1_1_STATE = "running" ]; then \
		echo "Stopping $(PRU1_1)"; \
		echo stop > $(PRU1_1)/state; \
	fi


.PHONY: all clean

# Remove the $(OUT_DIR) directory
clean:
	rm -rf $(OUT_DIR)
	rm -rf $(COMMON_OBJS)

# Includes the dependencies that the compiler creates (-ppd and -ppa flags)
-include $(OBJS:%.o=%.pp)
