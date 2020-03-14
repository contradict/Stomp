# This makefile uses https://github.com/sudar/Arduino-Makefile
# The paths below assume you have built arduino from source using these
# instructions https://github.com/arduino/Arduino/wiki/Building-Arduino
# Insted of installing the resulting .tar.gz, I just use the path to the built
# artifacts here

ARDUINO_DIR   = ${HOME}/src/Arduino/build/linux/work
ARDMK_DIR     = ${HOME}/src/Arduino-Makefile
BOARD_TAG     = mega
BOARD_SUB     = atmega2560
MONITOR_PORT  = /dev/ttyACM0

CXXFLAGS      = -Wall -DSERIAL_TX_BUFFER_SIZE=256 -fno-threadsafe-statics
CXXFLAGS_STD  = -std=c++11

LDFLAGS = -Wl,-Map,chomp.map

ARDUINO_LIBS = I2C MPU6050
USER_LIB_PATH := $(realpath ../libraries)

include ${ARDMK_DIR}/Arduino.mk

selfright.pdf: selfright.dot
	dot -Tpdf -o $@ $<
