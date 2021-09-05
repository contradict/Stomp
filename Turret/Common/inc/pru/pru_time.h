#ifndef _PRU_TIME_H_
#define _PRU_TIME_H_

#include <stdint.h>

void pru_time_init();
void pru_time_update();
   
// return the elapsed microseconds since initialization

uint32_t pru_time_gettime();

// return the elapsed microseconds since update call

uint32_t pru_time_dt();

#endif /* _PRU_TIME_H_ */
