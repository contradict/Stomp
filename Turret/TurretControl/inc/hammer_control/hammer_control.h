#pragma once

// 
// Shared between ARM and PRU code
//

enum state_change_reason
{
    timeout,
    hammer_angel_greater,
    hammer_angel_less,
    hammer_energy_greater,
    hammer_velocity_less,
    retract_pressure_greater
};
