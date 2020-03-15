#pragma once
#include <stddef.h>
#include "messages.h" // msg_id

void telem_init();
void telem_publish(msg_id id, char* data, size_t size);