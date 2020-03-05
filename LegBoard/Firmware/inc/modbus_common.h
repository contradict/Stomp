#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "joint.h"

#define CURL_BASE ((JOINT_CURL + 1) * 0x100)
#define SWING_BASE ((JOINT_SWING + 1) * 0x100)
#define LIFT_BASE ((JOINT_LIFT + 1) * 0x100)
    
uint16_t return_context(void *context);
bool return_context_bool(void *context);
void save_to_context(void *context, uint16_t value);
void save_to_context_bool(void *context, bool value);
