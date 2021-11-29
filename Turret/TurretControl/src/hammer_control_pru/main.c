//
// This program runs on the PRU.  It initializes everything and then
// waits around to receive messages from the ARM processor.  This
// program also calls the hammer controller update method.
//
// Key Messages
//
// SENS - A message from the Sensor Controller.  Contains hammer and
//        turret angles and velocities as well as throw and retract
//        pressure reading.  Also contains hammer energy calculations
//        performed on ARM, derived from sensor readings.
//
// CONF - A message from either the Turret Controller or the Telemetry
//        Controller, initializing or updating global configuration
//        values used in controlling the hammer state machine
//
// THRW - Throw the hammer message
//
// Output Pins
//
// BBAI Header Pin P8.14 - PRU Heartbeat
// BBAI Header Pin P8.16 - unused
// BBAI Header Pin P8.18 - sensor message received
// BBAI Header Pin P8.15 - PRU 1 eCap PWM output
// 
// Input Pins
//

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <pru_cfg.h>
#include <pru_ecap.h>
#include <pru_intc.h>
#include <rsc_types.h>
#include <pru_rpmsg.h>

#include "pru_util.h"
#include "pru_time.h"

#include "resource_table_1.h"

#include "hammer_control_pru/hammer_control_pru.h"

#define LOGGING_ENABLED 1

volatile register uint32_t __R30;
volatile register uint32_t __R31;

// -----------------------------------------------------------------------------
// file scope consts
// -----------------------------------------------------------------------------

static const uint32_t k_message_buffer_max_len = 496;
static const uint32_t k_message_recv_delay_dt_max = 50000000;

static const uint32_t k_heartbeat_dt = 500000;

// The PRU-ICSS system events used for RPMsg are defined in the Linux 
// device tree
//
// PRU0 uses system event 16 (To ARM) and 17 (From ARM)
// PRU1 uses system event 18 (To ARM) and 19 (From ARM)

static const uint32_t k_pru_to_host_event = 18;
static const uint32_t k_pru_from_host_event = 19;

// Host interrupt 0 is reported on pin 30 (of register __R31)
// Host interrupt 1 is reported on pin 31 (of register __R31)
//
// PRU0 uses interrupt 0
// PRU1 uses interrupt 1
//
// see 30.2.6.2 of AM572x SitaraTM Processors Silicon Revision 2.0, 1.1

static const uint32_t k_host_interrupt_1_bit = (uint32_t)(0x1<<31);

static int k_message_type_strlen = 4;
static char* k_message_type_exit = "EXIT";
static char* k_message_type_sync = "SYNC";
static char* k_message_type_sens = "SENS";
static char* k_message_type_conf = "CONF";
static char* k_message_type_logm = "LOGM";
static char* k_message_type_throw = "THRW";

#if LOGGING_ENABLED
static char s_log_message_buffer[512];
#endif

// directly write to gpio8_15 for heartbeat

static uint32_t *k_gpio8OutputEnable = (uint32_t *)0x48053134;
static uint32_t *k_gpio8ClearDataOut = (uint32_t *)0x48053190;
static uint32_t *k_gpio8SetDataOut = (uint32_t *)0x48053194;

static const uint32_t k_heartbeatPin = (0x1 << 15);

// -----------------------------------------------------------------------------
// states
// -----------------------------------------------------------------------------

enum state
{
    init,
    sync,
    safe,
    armed,
    active,
    invalid = -1
};

// -----------------------------------------------------------------------------
// file scope statics
// -----------------------------------------------------------------------------

static enum state s_state;
static uint32_t s_state_start_time;

static struct pru_rpmsg_transport s_transport;

static char s_send_message_buffer[496];
static char s_recv_message_buffer[496];

static uint16_t s_rpmsg_arm_addr;
static uint16_t s_rpmsg_pru_addr;

static int32_t s_throw_desired_intensity = 10;
static uint32_t s_message_last_recv_time = 0;

static uint32_t s_heartbeat_start_time;

static uint8_t s_comms_connected = 0;
static uint8_t s_radio_weapon_enabled = 0;
static uint8_t s_sync_message_received;
static uint8_t s_exit_message_received;

// -----------------------------------------------------------------------------
//  forward decl of internal methods
// -----------------------------------------------------------------------------

void init_syscfg();
void init_intc();
void init_rpmsg();
void init_gpio();
void init_heartbeat();

void update();
void update_comms();
void update_heartbeat();

void set_state(enum state);

void report_comms_up();
void report_comms_down();

int8_t is_comms_connected();
int8_t is_weapon_enabled();

void recv_sync_message(char *sync_message_buffer);
void recv_exit_message(char *exit_message_buffer);
void recv_conf_message(char *conf_message_buffer);
void recv_sens_message(char *sens_message_buffer);
void recv_throw_message(char *throw_message_buffer);

void send_swng_message();
void send_log_message(char *message);

int8_t check_for_arm_sync();
int8_t check_for_arm_exit();


// -----------------------------------------------------------------------------
//  Main
// -----------------------------------------------------------------------------

void main(void)
{
    // set initial state and init

    s_state = invalid;
    set_state(init);

    // execute forever

    while (1) 
    {
        update();
    }
}

// -----------------------------------------------------------------------------
//  State Machine Methods (update and set_state)
// -----------------------------------------------------------------------------

void update()
{
    pru_time_update();

    // update state

    while (1)
    {
        enum state prev_state = s_state;

        switch (s_state)
        {
            case init:
                {
                    set_state(sync);
                }
                break;

            case sync:
                {
                    if (check_for_arm_sync())
                    {
                        set_state(safe);
                    }
                }
                break;

            case safe:
                {
                    if (check_for_arm_exit())
                    {
                        set_state(sync);
                    }
                    else if (is_comms_connected() && is_weapon_enabled())
                    {
                        set_state(armed);
                    }
                }
                break;

            case armed:
                {
                    if (!is_comms_connected() || !is_weapon_enabled())
                    {
                        set_state(safe);
                    }
                    else if (check_for_arm_exit())
                    {
                        set_state(sync);
                    }
                }
                break;

            case active:
                {
                    if (!is_comms_connected() || !is_weapon_enabled())
                    {
                        set_state(safe);
                    }
                    else if (hammer_control_is_swing_complete())
                    {
                        set_state(armed);
                    }
                }
                break;
        }

        if (s_state == prev_state)
        {
            break;
        }
    }

    // now that state is updated, distribut the rest of the updates
    
    update_heartbeat();
    update_comms();

    hammer_control_update();
}

// -----------------------------------------------------------------------------
// Update Methods
// -----------------------------------------------------------------------------

void update_comms()
{
    //  Check to see if we have an rpmsg.  Keep track of how long it has been
    //  and if we wait too long, then consider comms down

    if (__R31 & k_host_interrupt_1_bit)
    {
        // Tell interrupt controller to clear interrupt from host 
        
        CT_INTC.SICR_bit.STATUS_CLR_INDEX = k_pru_from_host_event;

        // Go through all the messages queued
        
        uint16_t message_len;

        while (pru_rpmsg_receive(&s_transport, &s_rpmsg_arm_addr, &s_rpmsg_pru_addr, s_recv_message_buffer, &message_len) == PRU_RPMSG_SUCCESS) 
        {
            if (strncmp(s_recv_message_buffer, k_message_type_conf, k_message_type_strlen) == 0)
            {
                recv_conf_message(s_recv_message_buffer);
            }
            else if (strncmp(s_recv_message_buffer, k_message_type_sens, k_message_type_strlen) == 0)
            {
                recv_sens_message(s_recv_message_buffer);
            }
            else if (strncmp(s_recv_message_buffer, k_message_type_sync, k_message_type_strlen) == 0)
            {
                recv_sync_message(s_recv_message_buffer);
            }
            else if (strncmp(s_recv_message_buffer, k_message_type_exit, k_message_type_strlen) == 0)
            {
                recv_exit_message(s_recv_message_buffer);
            }
            else if (strncmp(s_recv_message_buffer, k_message_type_throw, k_message_type_strlen) == 0)
            {
                recv_throw_message(s_recv_message_buffer);
            }
        }

        // Update time that we received a message and make sure comms are considered up
        
        s_message_last_recv_time = pru_time_gettime();
        report_comms_up();
    }
    else
    {
        //  If it has been too long since we have received a message, consider comms down

        uint32_t current_time = pru_time_gettime();

        if (current_time - s_message_last_recv_time > k_message_recv_delay_dt_max)
        {
            report_comms_down();
        }
    }
}

void update_heartbeat()
{
    static uint8_t heartbeat_status = 0;

    uint32_t time = pru_time_gettime();

    if (time - s_heartbeat_start_time > k_heartbeat_dt)
    {
        s_heartbeat_start_time = time;

        uint32_t pins_high = 0x00;
        uint32_t pins_low = 0x00;

        if (heartbeat_status == 0)
        {
            heartbeat_status = 1;
            pins_high = k_heartbeatPin;
        }
        else
        {
            heartbeat_status = 0;
            pins_low = k_heartbeatPin;
        }

        (*k_gpio8ClearDataOut) = pins_low;
        (*k_gpio8SetDataOut) = pins_high;
    }
}

// -----------------------------------------------------------------------------
// Set State - tansition from one state to another
// -----------------------------------------------------------------------------

void set_state(enum state new_state)
{
    if (s_state == new_state)
    {
        return;
    }

    // do work on leaving state

    switch (s_state)
    {
        case active:
            {
                send_swng_message();
            }
            break;

        default:
            break;
    }

    // update to new state

    s_state = new_state;
    s_state_start_time = pru_time_gettime();

    // do work on entering state
    
    switch (s_state)
    {
        case init:
            {
                // init the pru time module

                pru_time_init();

                // init all our interal systems
                
                init_syscfg();
                init_intc();
                init_rpmsg();
                init_gpio();
                init_heartbeat();
        
                hammer_control_init();
            }
            break;

        case active:
            {
                // throw the hammer!!!
                
                hammer_control_trigger_throw(s_throw_desired_intensity);
            }

        default:
            break;
    }
}

// -----------------------------------------------------------------------------
// Message sending and receiving methods
// -----------------------------------------------------------------------------

void recv_sync_message(char *sens_message_buffer)
{
    s_sync_message_received = 1;

#if LOGGING_ENABLED

    char* debug_message = s_log_message_buffer;

    memset(debug_message, 0, 512);
    memcpy(debug_message, "sync message received", 21); 
    debug_message += 21;

    *debug_message = '\n';
    debug_message++;
    *debug_message = 0;

    send_log_message(s_log_message_buffer);

#endif
}

void recv_exit_message(char *sens_message_buffer)
{
    s_exit_message_received = 1;
}

enum swing_type
{
    throw_retract,
    retract_only,

    swing_unknown,
};

void recv_throw_message(char *throw_message_buffer)
{
    enum swing_type type = swing_unknown;

    // parse the throw message
    
    char* token = strtok(throw_message_buffer, ":");

    // confirm it is throw message

    if (strncmp(token, k_message_type_throw, k_message_type_strlen) != 0)
    {
        return;
    }

    while (token != NULL)
    {
        if (strcmp(token, "INTENSITY") == 0)
        {
            s_throw_desired_intensity = atoi(strtok(NULL, ":"));
        }
        else if (strcmp(token, "TYPE") == 0)
        {
            token = strtok(NULL, ":");

            if (strcmp(token, "THROW") == 0)
            {
                type = throw_retract;
            }
            else if (strcmp(token, "RETRACT") == 0)
            {
                type = retract_only;
            }
        }

        token = strtok(NULL, ":");
    }

    //  If we are in armed state, that means we are ready and able to throw.
    //  tell the hammer swing state machine to throw, by setting our
    //  state to active

    if (s_state == armed)
    {
        // TODO: Handle both types of swing
        set_state(active);
    }

#if LOGGING_ENABLED

    char* debug_message = s_log_message_buffer;
    char* number_string;
    int number_string_len;

    memset(debug_message, 0, 512);

    memcpy(debug_message, "swing hammer with angle: ", 25); 
    debug_message += 25;

    number_string = pru_util_itoa(s_throw_desired_intensity, 10);
    number_string_len = strlen(number_string);

    memcpy(debug_message, number_string, number_string_len);
    debug_message += number_string_len;

    *debug_message = '\n';
    debug_message++;
    *debug_message = 0;

    send_log_message(s_log_message_buffer);

#endif
}

void recv_conf_message(char *conf_message_buffer)
{
    // parse the conf message
    
    char* token = strtok(conf_message_buffer, ":");

    // confirm it is CONF message

    if (strncmp(token, k_message_type_conf, k_message_type_strlen) != 0)
    {
        return;
    }

    while (token != NULL)
    {
        if (strcmp(token, "TA") == 0)
        {
            g_max_throw_angle = atoi(strtok(NULL, ":"));
        }
        else if (strcmp(token, "RA") == 0)
	{
            g_min_retract_angle = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "RFP") == 0)
	{
    	    g_retract_fill_pressure = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "BEV") == 0)
	{
            g_brake_exit_velocity = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "EBA") == 0)
	{
            g_emergency_brake_angle = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "VCDT") == 0)
	{
            g_valve_change_dt = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "TPDT") == 0)
	{
            g_max_throw_pressure_dt = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "RPDT") == 0)
	{
            g_max_retract_pressure_dt = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "TEDT") == 0)
        {
            g_max_throw_expand_dt = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "REDT") == 0)
	{
            g_max_retract_expand_dt = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "RBDT") == 0)
	{
            g_max_retract_break_dt = atoi(strtok(NULL, ":"));
	}
        else if (strcmp(token, "RSDT") == 0)
	{
            g_max_retract_settle_dt = atoi(strtok(NULL, ":"));
	}

        token = strtok(NULL, ":");
    }

    hammer_control_config_update();

#if LOGGING_ENABLED

    char* debug_message = s_log_message_buffer;
    char* number_string;
    int number_string_len;

    memset(debug_message, 0, 512);

    memcpy(debug_message, "parsed conf message: ", 21); 
    debug_message += 21;

    memcpy(debug_message, "RPDT=", 5); 
    debug_message += 5;

    number_string = pru_util_itoa(g_max_retract_pressure_dt, 10);
    number_string_len = strlen(number_string);

    memcpy(debug_message, number_string, number_string_len);
    debug_message += number_string_len;

    *debug_message = '\n';
    debug_message++;
    *debug_message = 0;

    send_log_message(s_log_message_buffer);

#endif
}

void recv_sens_message(char *sens_message_buffer)
{
    // parse the sensor message
    
    char* token = strtok(sens_message_buffer, ":");

    // confirm it is SENS message

    if (strncmp(token, k_message_type_sens, k_message_type_strlen) != 0)
    {
        return;
    }

    token = strtok(NULL, ":");

    while (token != NULL)
    {
        if (strcmp(token, "HA") == 0)
        {
            g_hammer_angle = atoi(strtok(NULL, ":"));
            g_hammer_velocity = atoi(strtok(NULL, ":"));
        }
        else if (strcmp(token, "TA") == 0)
        {
            g_turret_angle = atoi(strtok(NULL, ":"));
            g_turret_velocity = atoi(strtok(NULL, ":"));
        }
        else if (strcmp(token, "TP") == 0)
        {
            g_throw_pressure = atoi(strtok(NULL, ":"));
        }
        else if (strcmp(token, "RP") == 0)
        {
            g_retract_pressure = atoi(strtok(NULL, ":"));
        }

        token = strtok(NULL, ":");
    }
    
    // TODO: Add these variables to the message 
   
    g_hammer_energy = 0;
    g_available_break_energy = 0;

    // send a debug message back to ARM so we can debug what is going
    // on here
    
#if LOGGING_ENABLED

    char* debug_message = s_log_message_buffer;
    char* number_string;
    int number_string_len;

    memset(debug_message, 0, 512);

    memcpy(debug_message, "parsed sens message: ", 21); 
    debug_message += 21;

    memcpy(debug_message, "HA=", 3); 
    debug_message += 3;

    number_string = pru_util_itoa(g_hammer_angle, 10);
    number_string_len = strlen(number_string);

    memcpy(debug_message, number_string, number_string_len);
    debug_message += number_string_len;

    memcpy(debug_message, " PRD=", 5); 
    debug_message += 5;

    number_string = pru_util_itoa(g_hammer_angle, 10);
    number_string_len = strlen(number_string);

    memcpy(debug_message, number_string, number_string_len);
    debug_message += number_string_len;

    *debug_message = '\n';
    debug_message++;
    *debug_message = 0;

    send_log_message(s_log_message_buffer);

#endif
}

void send_swng_message()
{
    if (s_state == sync || s_state == init)
    {
        return;
    }

    uint32_t timestamp = pru_time_gettime() - s_state_start_time;

    // Can't use sprintf beacuse it doesn't fit in PRU memory
    // so used an custom itoa impl

    char* message = s_send_message_buffer;

    char* timestamp_string = pru_util_itoa(timestamp, 10);
    int timestamp_string_len = strlen(timestamp_string);
    memcpy(message, timestamp_string, timestamp_string_len);

    message += timestamp_string_len;
    *message = ':';
    message++;

    char* test_string = "SWNG:";
    int test_string_len = strlen(test_string);
    memcpy(message, test_string, test_string_len);

    message += test_string_len;
    *message = '\n';
    message++;
    *message = 0;

    // send the message out
    
    pru_rpmsg_send(&s_transport, s_rpmsg_pru_addr, s_rpmsg_arm_addr, s_send_message_buffer, k_message_buffer_max_len);
}

void send_log_message(char *log_message)
{
    uint32_t timestamp = pru_time_gettime() - s_state_start_time;

    if (strlen(log_message) >= (k_message_buffer_max_len-20))
    {
        return;
    }

    char* message = s_send_message_buffer;

    char* timestamp_string = pru_util_itoa(timestamp, 10);
    int timestamp_string_len = strlen(timestamp_string);
    memcpy(message, timestamp_string, timestamp_string_len);

    message += timestamp_string_len;
    *message = ':';
    message++;

    memcpy(message, k_message_type_logm, k_message_type_strlen);
    message += k_message_type_strlen;
    *message = ':';
    message++;

    memcpy(message, log_message, strlen(log_message));
    message += strlen(log_message);
    *message = '\n';
    message++;
    *message = 0;

    pru_rpmsg_send(&s_transport, s_rpmsg_pru_addr, s_rpmsg_arm_addr, s_send_message_buffer, k_message_buffer_max_len);
}

int8_t check_for_arm_sync()
{
    uint8_t received_sync = s_sync_message_received;

    if (s_sync_message_received)
    {
        s_sync_message_received = 0;
    }

    return received_sync;
}

int8_t check_for_arm_exit()
{
    uint8_t received_exit = s_exit_message_received;

    if (s_exit_message_received)
    {
        s_exit_message_received = 0;
    }

    return received_exit;
}

// -----------------------------------------------------------------------------
//  Init Methods
// -----------------------------------------------------------------------------

void init_syscfg()
{
    // Allow OCP master port access by the PRU so the PRU can read external memories

    CT_CFG.SYSCFG_bit.STANDBY_INIT = 0x0;
}

void init_intc()
{
    // Tell interrupt controller to clear interrupt from host 

    CT_INTC.SICR_bit.STATUS_CLR_INDEX = k_pru_from_host_event;
}

void init_rpmsg()
{
    volatile uint8_t *status;

    // Make sure the Linux drivers are ready for RPMsg communication 
    // Used to make sure the Linux drivers are ready for RPMsg communication
    // Found at linux-x.y.z/include/uapi/linux/virtio_config.h

    status = &resourceTable.rpmsg_vdev.status;
    while (!(*status & 0x04));

    // Initialize the RPMsg transport structure 

    pru_rpmsg_init(&s_transport, 
            &resourceTable.rpmsg_vring0, 
            &resourceTable.rpmsg_vring1, 
            k_pru_to_host_event, 
            k_pru_from_host_event);

    // Create the RPMsg channel between the PRU and ARM user space using the transport structure. 
    // Using the name 'rpmsg-pru' will probe the rpmsg_pru driver found
    // at linux-x.y.z/drivers/rpmsg/rpmsg_pru.c

    uint32_t message_status;
    do
    {
         message_status = pru_rpmsg_channel(RPMSG_NS_CREATE, 
                 &s_transport, 
                 "rpmsg-pru", 
                 "PRUMessages", 
                 31);
    } while (message_status != PRU_RPMSG_SUCCESS);

    // init message send time

    report_comms_down();
    
    s_sync_message_received = 0;
    s_exit_message_received = 0;

    // Clear out the message buffer
    
    memset(s_send_message_buffer, 0, k_message_buffer_max_len);
}

void init_gpio()
{
    // For the output enable register, 0 means eanbled and 1 is disabled
    // clear to zero just the 4 pins we need as output, leaving the rest 
    // as they are

    uint32_t enableOutput = ~k_heartbeatPin;
    (*k_gpio8OutputEnable) &= enableOutput;

    uint32_t pins_low = k_heartbeatPin;
    (*k_gpio8ClearDataOut) = pins_low;
}

void init_heartbeat()
{
    s_heartbeat_start_time = pru_time_gettime();
}

// -----------------------------------------------------------------------------
// Utility Methods
// -----------------------------------------------------------------------------

void report_comms_up()
{
    s_comms_connected = 1;
}

void report_comms_down()
{
    s_comms_connected = 0;
}

int8_t is_comms_connected()
{
    return s_comms_connected;
}

int8_t is_weapon_enabled()
{
    // Need to really implement this
    s_radio_weapon_enabled = 1;
    return s_radio_weapon_enabled;
}
