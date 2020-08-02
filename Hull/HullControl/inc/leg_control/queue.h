#pragma once
#include <stdint.h>
#include <stddef.h>

#include "ringbuf.h"

struct queue {
    ringbuf_t *ringbuf;
    uint8_t *buffer;
};


void create_queue(int nproducers, size_t bufsize, struct queue *q);
