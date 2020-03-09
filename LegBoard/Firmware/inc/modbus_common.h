#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "joint.h"

#define CURL_BASE ((JOINT_CURL + 1) * 0x100)
#define SWING_BASE ((JOINT_SWING + 1) * 0x100)
#define LIFT_BASE ((JOINT_LIFT + 1) * 0x100)

#define ENFIELD_CONTEXT_VALUE(j, w, r) ((void *)(((j&3)<<30) | ((w&0xff)<<8) | (r&0xff)))

