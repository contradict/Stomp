#pragma once

#include <sys/time.h>

time_t time_diff_msec(struct timeval t0, struct timeval t1);
