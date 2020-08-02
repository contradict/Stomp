#include <stdlib.h>

#include "leg_control/queue.h"


void create_queue(int nproducers, size_t bufsize, struct queue *q)
{
    size_t ringbuf_obj_size;
    ringbuf_get_sizes(nproducers, &ringbuf_obj_size, NULL);
    q->ringbuf = malloc(ringbuf_obj_size);
    q->buffer = malloc(bufsize);
    ringbuf_setup(q->ringbuf, nproducers, bufsize);
}
