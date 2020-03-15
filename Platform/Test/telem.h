/*
    telem.h
    Utility library for sending out telemetry. Abstracts away the details of the telemetry
    transport layer.
*/

#pragma once
#include <stddef.h>
#include "messages.h" // msg_id

// Handle any stateful initialization of the transport layer
void telem_init();

// Publish a message (should be a structure defined in messages.h)
// NOTE! This is NOT THREAD SAFE currently
void telem_publish(msg_id id, char* data, size_t size);