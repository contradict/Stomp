#pragma once

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

void rpmsg_init();
void rpmsg_update();

void rpmsg_send_swng_message();
void rpmsg_send_log_message(char *log_message);

int8_t rpmsg_is_connected();
int8_t rpmsg_check_for_arm_sync();
int8_t rpmsg_check_for_arm_exit();


