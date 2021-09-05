
#include <stdint.h>
#include <pru_ctrl.h>

#include "pru_time.h"

// file scope consts

const static uint32_t k_cycles_to_micros = 200;

// file scope variables

static uint32_t s_time_micros;
static uint32_t s_time_remainder_cycles;
static uint32_t s_delta_time_micros;

// public methods

void pru_time_init()
{
    s_time_micros = 0;
    s_time_remainder_cycles = 0;

    s_delta_time_micros = 0;

    // reset and start hardware cycle counter

    PRU1_CTRL.CONTROL_bit.COUNTER_ENABLE = 0x0;
    PRU1_CTRL.CYCLE_bit.CYCLECOUNT = 0x00000000;
    PRU1_CTRL.CONTROL_bit.COUNTER_ENABLE = 0x1;
}

void pru_time_update()
{
    uint32_t micros;
    uint32_t cycles = PRU1_CTRL.CYCLE_bit.CYCLECOUNT + s_time_remainder_cycles;

    // reset hardware cycle counter
    
    PRU1_CTRL.CONTROL_bit.COUNTER_ENABLE = 0x0;
    PRU1_CTRL.CYCLE_bit.CYCLECOUNT = 0x00000000;
    PRU1_CTRL.CONTROL_bit.COUNTER_ENABLE = 0x1;

    // calculate number microseconds since last reset, keeping
    // track of any remaining cycles below micorseconds

    micros =  cycles / k_cycles_to_micros;
    s_time_remainder_cycles = cycles - (micros * k_cycles_to_micros);

    // incrament elapsed microseconds and difference since last call to update
    
    s_time_micros += micros;
    s_delta_time_micros = micros;
}

// return the elapsed microseconds since initialization

uint32_t pru_time_gettime()
{
    return s_time_micros;
}

// return the elapsed microseconds since update call

uint32_t pru_time_dt()
{
    return s_delta_time_micros;
}

