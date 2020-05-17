#pragma once
#include <stdbool.h>
#include <stdint.h>

#define ENFIELD_CONTEXT_VALUE(j, w, r) ((void *)(((j&3)<<30) | ((w&0xff)<<8) | (r&0xff)))

int return_context(void *context, uint16_t *v);
int return_context_bool(void *context, bool *v);
int save_to_context(void *context, uint16_t value);
int save_to_context_bool(void *context, bool v);
