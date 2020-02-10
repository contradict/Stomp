#pragma once
#include <Stream.h>
#include "telem.h"

class TelemetryMessageStream: public Stream {
    String line;
    public:
    size_t write(const uint8_t *buffer, size_t size) {
        if(line.length() + size > 128) {
            debug_print(line);
            line = String((char *)buffer);
        } else {
            line += (char *)buffer;
        }
        if(buffer[size-1] == '\r' || buffer[size-1] == '\n') {
            debug_print(line);
        }
        return size;
    }
    size_t write(uint8_t) {return 0;};
    int available() {return 0;};
    int read() {return 0;};
    int peek() {return 0;};
    void flush() {};
};

