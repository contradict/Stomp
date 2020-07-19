/*
struct AutoFireParameters {
    int32_t xtol, ytol;
    int32_t max_omegaZ;   // rad/s * 2048 = 50 deg/sec
    uint32_t autofire_telem_interval;
} __attribute__((packed));

static struct AutoFireParameters params;
static uint32_t last_autofire_telem = 0;

static struct AutoFireParameters EEMEM saved_params_old = {
    .xtol = 200,   // currently unused, front of box is depth
    .ytol = 200,
    .max_omegaZ = 1787,   // rad/s * 2048 = 50 deg/sec
    .autofire_telem_interval = 100000,
};

static void saveAutoFireParameters(void);

int32_t swingDuration(int16_t hammer_intensity) {
    int16_t hammer_angle = HAMMER_INTENSITIES_ANGLE[hammer_intensity];
    int32_t x=(40L-hammer_angle);
    return 230L + (3L*x*x*x)/1024L;
}

bool omegaZLockout(int32_t *omegaZ) {
    *omegaZ = 0;
    int16_t rawOmegaZ;
    bool imu_valid = getOmegaZ(&rawOmegaZ);
    *omegaZ = (int32_t)rawOmegaZ*35L/16L;
    return imu_valid && abs(*omegaZ)>params.max_omegaZ;
}

int8_t nsteps=3;
enum AutoFireState updateAutoFire(const Track &tracked_object,
                           int16_t depth, int16_t hammer_intensity) {
    int32_t omegaZ=0;
    uint32_t now = micros();
    bool hit = false;
    bool lockout = omegaZLockout(&omegaZ);
    bool valid = tracked_object.valid(now);
    int32_t swing = 0;
    int32_t x=0, y=0;
    if(valid && !lockout) {
        swing=swingDuration(hammer_intensity)*1000;
        x=tracked_object.x;
        y=tracked_object.y;
        int32_t dt=swing/nsteps;
        for(int s=0;s<nsteps;s++) {
            tracked_object.project(dt, dt*omegaZ/1000000, &x, &y);
        }
        hit = (x>0) && (x/16<depth) && abs(y/16)<params.ytol;
    }
    enum AutoFireState st;
    if(lockout) st =     AF_OMEGAZ_LOCKOUT;
    else if(!valid) st = AF_NO_TARGET;
    else if(!hit) st =   AF_NO_HIT;
    else st =            AF_HIT;
    if(now - last_autofire_telem > params.autofire_telem_interval) {
        sendAutofireTelemetry(st, swing, x/16, y/16);
    }
    return st;
}


void saveAutoFireParameters(void) {
    eeprom_write_block(&params, &saved_params_old, sizeof(struct AutoFireParameters));
}

void restoreAutoFireParameters(void) {
    eeprom_read_block(&params, &saved_params_old, sizeof(struct AutoFireParameters));
}
*/

//  ====================================================================
//
//  Auto Fire
//
//  The Auto Fire object is owned by the Hammer Controller
//  and has only 1 job...
//
//  Figure out when target is in the killzone and tell the hammer
//  controller when to fire
//
//  ====================================================================


#include <Arduino.h>

#include "turretController.h"
#include "autofire.h"
#include "sbus.h"
#include "telemetryController.h"
#include "fixedpoint.h"
#include "radioController.h"

#include "autofire.h"

//  ====================================================================
//
//  External references
//
//  ====================================================================

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static struct AutoFire::Params EEMEM s_savedParams = 
{
    .xtol = 200,   // currently unused, front of box is depth
    .ytol = 200,
    .max_omegaZ = 1787,   // rad/s * 2048 = 50 deg/sec
};

//  ====================================================================
//
//  Constructors
//
//  ====================================================================

//  ====================================================================
//
//  Public API methods
//
//  ====================================================================

void AutoFire::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void AutoFire::Update() 
{
    m_lastUpdateTime = micros();

    while(true)
    {
        autoFireState prevState = m_state;

        switch (m_state)
        {
            case EInit:
            {
                setState(ESafe);
            }
            break;

            case ESafe:
            {
                //  Stay in safe mode for a minimum of k_safeStateMinDt

                if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && Radio.IsNominal())
                {
                    if (isAutoFireEnabled())
                    {
                        setState(ENoTarget);
                    }
                    else
                    {
                        setState(EDisabled);
                    }                    
                }
            }
            break;     

            case EDisabled:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (isAutoFireEnabled())
                {
                    setState(ENoTarget);
                }
            }
            break;

            case ENoTarget:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!isAutoFireEnabled())
                {
                    setState(EDisabled);
                }
            }
            break;

            default:
            break;
        }

        //  No more state changes, move on
        
        if (m_state == prevState)
        {
            break;
        }
    }

    //  Now that the state is stable, do our updateDesiredSpeed

}

void AutoFire::SetParams(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz)
{
    m_params.xtol = p_xtol;
    m_params.ytol = p_ytol;
    m_params.max_omegaZ = p_max_omegaz;

    saveParams();
}

void AutoFire::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct AutoFire::Params));
}

void AutoFire::SendTelem()
{
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void AutoFire::setState(autoFireState p_state)
{
    if (m_state == p_state)
    {
        return;
    }

    //  exit state transition

    switch (m_state)
    {
        default:
        break;
    }

    m_state = p_state;
    m_stateStartTime = m_lastUpdateTime;
;

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
        }
        break;

        default:
        break;
    }
}

void AutoFire::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct AutoFire::Params));
}