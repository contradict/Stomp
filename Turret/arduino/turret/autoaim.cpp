#include <Arduino.h>
#include "autoaim.h"
#include "autofire.h"
#include "imu.h"
#include "telem.h"
#include "fixedpoint.h"


//  ====================================================================
//
//  Global Variables
//
//  ====================================================================

//  ====================================================================
//
//  Global constants
//
//  ====================================================================

//  ====================================================================
//
//  Utility structures
//
//  ====================================================================

struct AutoAimParameters 
{
    int32_t steer_p;
    int32_t steer_d;
    int32_t steer_max;
    int32_t gyro_gain;
    uint32_t autoaimTelemInterval;
} __attribute__((packed));

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static int32_t s_desiredTurretSpeed =0;

static struct AutoAimParameters s_autoAimParams;
static uint32_t s_lastAutoaimTelem = 0;

static struct AutoAimParameters EEMEM s_savedAutoAimParams = 
{
    .steer_p = 3000,
    .steer_d = 0,
    .steer_max = 600,
    .gyro_gain = 0,
    .autoaimTelemInterval = 50000,
};

//  ====================================================================
//
//  External references
//
//  ====================================================================

//  ====================================================================
//
//  Forward references to internal (private) methods
//
//  ====================================================================

static void saveAutoAimParameters(void);

//  ====================================================================
//
//  Global (public) methods
//
//  ====================================================================

enum AutoAimState updateAutoAim(const Track &p_trackedObject) 
{
    uint32_t now = micros();
 
    enum AutoAimState state = AA_NO_TARGET;
    s_desiredTurretSpeed = 0;
    
    //  Calculate the speed at which the turret should turn to place
    //  the tracked object in line with the swing of the hammer.
    //
    //  This caclulation ignores all other turret motion factors, such as
    //  the angular velocity of the hull or controller input.  Simply
    //  caclulate the turret rotation relative to a unmoving base that                                  
    //  will align the hammer to the target

    if (p_trackedObject.valid(now))
    {
        //
        //  Magically convert from where the target is and how fast it is moving, into 
        //  the speed we want the turret to be turning
        //

        state == AA_TRACKING_TARGET;    
        s_desiredTurretSpeed = FROM_FP_32x14(-p_trackedObject.vtheta() * getTmpAdjustment());

        /*
        //  This is more like the real alogrithm, but the above is fine for now
        int16_t steer_bias;

        int32_t bias;

        int32_t theta = p_trackedObject.angle();
        int32_t vtheta = p_trackedObject.vtheta();

        //  Convert from the angle between forward (hammer axis) and target
        //  in to the speed we need to run the motor at 
        bias  = FROM_FP_32x14(s_autoAimParams.steer_p * (0 - theta));
        bias += FROM_FP_32x14(-s_autoAimParams.steer_d * vtheta)

        int16_t omegaZ = 0;
        if(getOmegaZ(&omegaZ)) 
        {
            bias += -s_autoAimParams.gyro_gain*omegaZ/1024;
        }

        steer_bias  = constrain(bias, -s_autoAimParams.steer_max, s_autoAimParams.steer_max);
        */
    }

    if (now - s_lastAutoaimTelem > s_autoAimParams.autoaimTelemInterval) 
    {
        sendAutoAimTelemetry(0, 0, 0, 0, 0);
    }

    return state;
}

int32_t desiredAutoAimTurretSpeed()
{
    return s_desiredTurretSpeed;
}

void setAutoAimParams(int16_t p_steer_p, int16_t p_steer_d, int16_t p_steer_max, int16_t p_gyro_gain)
{
    s_autoAimParams.steer_p = p_steer_p;
    s_autoAimParams.steer_d = p_steer_d;
    s_autoAimParams.gyro_gain = p_gyro_gain;
    s_autoAimParams.steer_max = p_steer_max;

    saveAutoAimParameters();
}

void restoreAutoAimParameters(void) {
    eeprom_read_block(&s_autoAimParams, &s_savedAutoAimParams, sizeof(struct AutoAimParameters));
}

//  ====================================================================
//
//  Internal (private) Methods
//
//  ====================================================================

void saveAutoAimParameters(void) {
    eeprom_write_block(&s_autoAimParams, &s_savedAutoAimParams, sizeof(struct AutoAimParameters));
}
