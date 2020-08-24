#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "rfd900x.h"

#include "telemetry.h"

#define COSMOS_OVERHEAD 3
#define COSMOS_TERMINATOR_SIZE 2

static const uint16_t g_cosmos_terminator = 0x6666;
static char g_buffer[MAX_MESSAGE_SIZE + COSMOS_OVERHEAD];

void telem_init()
{
    // Right now the only supported configuration is broadcasting over a
    // RFD900x radio to a Cosmos telemetry system, but if we wanted to switch
    // out either of those we could make all those changes internally without
    // needing to update client code.
    rfd900x_init();
}

void telem_publish(msg_id id, char* data, size_t size)
{
    printf("Received msg_id %d size %lu\n", id, size);
    // Construct a Cosmos terminated style packet. We assume/expect
    // that there is a matching Cosmos definition for every message
    // in messages.h
    g_buffer[0] = (char)id;
    memcpy(&g_buffer[1], data, size);
    memcpy(&g_buffer[size+1], &g_cosmos_terminator, COSMOS_TERMINATOR_SIZE);
    rfd900x_write(g_buffer, size+COSMOS_OVERHEAD);
}
