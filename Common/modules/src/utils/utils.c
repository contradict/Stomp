#include "utils/utils.h"
#include "sclog4c/sclog4c.h"

time_t time_diff_msec(struct timeval t0, struct timeval t1)
{
    return (t1.tv_sec - t0.tv_sec)*1000 + (t1.tv_usec - t0.tv_usec)/1000;
}
