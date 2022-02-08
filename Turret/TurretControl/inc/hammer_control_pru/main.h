#pragma once

//  enable logging from PRU to ARM via rpmsg

#define LOGGING_ENABLED 1

// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

void request_hammer_throw();
void request_hammer_retract();
