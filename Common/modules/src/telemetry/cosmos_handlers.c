#include <inttypes.h>
#include <unistd.h>
#include <lcm/lcm.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>

#include "rfd900x.h"
#include "cosmos_handlers.h"
#include "sclog4c/sclog4c.h"


// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const int message_buffer_size_max = 4096;

// -----------------------------------------------------------------------------
// file scope variables
// -----------------------------------------------------------------------------

static char s_message_buffer[4096];

// -----------------------------------------------------------------------------
// cosmos message handlers
// -----------------------------------------------------------------------------

void cosmos_handle()
{
    int num_bytes = rfd900x_read(s_message_buffer, message_buffer_size_max);

    if (num_bytes <= 0)
    {
       logm(SL4C_ERROR, "could not get valid cosmos message");   
       return;
    }
    // First byte is the id of the cmd message from cosmos
    // distribute message based on that id (and strip it off)

    int8_t message_id = (int8_t)s_message_buffer[0];

    switch (message_id)
    {
        case 1:
        {
            logm(SL4C_DEBUG, "Received Cosmos Hammer Parameters CMD");   
            hammer_parameters_handler(&s_message_buffer[1]);
        }
        break;

        default:
        break;
    }
}