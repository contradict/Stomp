#pragma once

// -----------------------------------------------------------------------------
// global variables
// -----------------------------------------------------------------------------


// -----------------------------------------------------------------------------
// public methods
// -----------------------------------------------------------------------------

void rpmsg_init();
void rpmsg_update();

void rpmsg_send_log_message(char *log_message);
void rpmsg_send_swing_message(int32_t swing_state_dt, int32_t trigger_value, int32_t trigger_limit, int8_t trigger_reason, int8_t swing_state_from, int8_t swing_state_to);

int8_t rpmsg_is_connected();
int8_t rpmsg_check_for_arm_sync();
int8_t rpmsg_check_for_arm_exit();


