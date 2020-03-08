//  ====================================================================
//
//  Auto Aim
//
//  The Auto Aim object is owned by the Turret Rotation Controllers
//  and has only 1 job...
//
//  Provides the desired angular velocity necessary to aim directly
//  at tracked target.
//
//  ====================================================================


#include <Arduino.h>

#include "turretController.h"
#include "autoaim.h"
#include "autofire.h"
#include "imu.h"
#include "sbus.h"
#include "telem.h"
#include "fixedpoint.h"

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

static struct AutoAim::Params EEMEM s_savedParams = 
{
    .steer_p = 3000,
    .steer_d = 0,
    .steer_max = 600,
    .gyro_gain = 0,
    .autoaimTelemInterval = 50000,
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

void AutoAim::Init()
{
    m_state = EInvalid;
    setState(EInit);
}

void AutoAim::Update() 
{
    uint32_t now = micros();


    while(true)
    {
        autoAimState prevState = m_state;

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

                if (now - m_stateStartTime > k_safeStateMinDt && isRadioConnected())
                {
                    if (isAutoAimEnabled())
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
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (isAutoAimEnabled())
                {
                    setState(ENoTarget);
                }
            }
            break;

            case ENoTarget:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (!isAutoAimEnabled())
                {
                    setState(EDisabled);
                }
                else if (Turret.GetCurrentTarget()->valid(now))
                {
                    setState(ETrackingTarget);
                }
            }
            break;

            case ETrackingTarget:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (!Turret.GetCurrentTarget()->valid(now))
                {
                    setState(ENoTarget);
                }
            }
            break;
        }

        //  No more state changes, move on
        
        if (m_state == prevState)
        {
            break;
        }
    }

    //  Now that the state is stable, do our updateDesiredSpeed

    updateDesiredTurretSpeed();
}

int32_t AutoAim::GetDesiredTurretSpeed()
{
    return m_desiredTurretSpeed;
}

void AutoAim::SetParams(int16_t p_steer_p, int16_t p_steer_d, int16_t p_steer_max, int16_t p_gyro_gain)
{
    m_params.steer_p = p_steer_p;
    m_params.steer_d = p_steer_d;
    m_params.gyro_gain = p_gyro_gain;
    m_params.steer_max = p_steer_max;

    saveParams();
}

void AutoAim::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct AutoAim::Params));
}

void AutoAim::SendTelem()
{
    if (m_state == ETrackingTarget && m_pTarget != nullptr)
    {
        sendAutoAimTelemetry(m_state, m_desiredTurretSpeed, m_pTarget->angle(), m_pTarget->vtheta(), 0, 0);
    }
    else
    {
        sendAutoAimTelemetry(m_state, m_desiredTurretSpeed, 0, 0, 0, 0);
    }
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void AutoAim::updateDesiredTurretSpeed()
{
    m_desiredTurretSpeed = 0;

    /*
    bool valid = tracked_object.valid(now);
    if(valid) {
        int32_t bias;
        int32_t theta = tracked_object.angle();
        int32_t vtheta = tracked_object.vtheta();
        bias  = params.steer_p * (0 - theta) / 16384L;
        bias += -params.steer_d * vtheta / 16384L;
        int16_t omegaZ = 0;
        if(getOmegaZ(&omegaZ)) {
            bias += -params.gyro_gain*omegaZ/1024;
        }
        *steer_bias  = clip(bias, -params.steer_max, params.steer_max);
        int32_t tracked_r = integer_sqrt(
            (tracked_object.x / 4L) * (tracked_object.x / 4L) +
            (tracked_object.y / 4L) * (tracked_object.y / 4L));
        int32_t tracked_vr = integer_sqrt(
            (tracked_object.vx / 4L) * (tracked_object.vx / 4L)+
            (tracked_object.vy / 4L) * (tracked_object.vy / 4L));
        bias  = params.drive_p * ((int32_t)depth*4L - tracked_r)/16384L;
        bias += -params.drive_d * tracked_vr * 4 / 16384L;
        *drive_bias  = clip(bias, -params.drive_max, params.drive_max);
        if(now - last_autodrive_telem > params.autodrive_telem_interval) {
            sendAutodriveTelemetry(*steer_bias,
                                   *drive_bias,
                                   clip(theta, -32768L, 32767L),
                                   clip(vtheta, -32768L, 32767L),
                                   clip(tracked_r / 4, -32768L, 32767L),
                                   clip(tracked_vr / 4, -32768L, 32767L));
        */

           /*
    Track p_trackedObject;

    uint32_t now = micros();
 
    enum AutoAimState state = AA_NO_TARGET;
    m_desiredTurretSpeed = 0;
    
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
    }

    if (now - s_lastAutoaimTelem > s_autoAimParams.autoaimTelemInterval) 
    {
        sendAutoAimTelemetry(0, 0, 0, 0, 0);
    }

    return state;
    */
}

void AutoAim::setState(autoAimState p_state)
{
    if (m_state == p_state)
    {
        return;
    }

    //  exit state transition

    switch (m_state)
    {
        case ETrackingTarget:
        {
            m_pTarget = nullptr;
        }
        break;
    }

    m_state = p_state;
    m_stateStartTime = micros();

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
            m_pTarget = nullptr;
        }
        break;

        case ETrackingTarget:
        {
            m_pTarget = Turret.GetCurrentTarget();
        }
        break;
    }
}

void AutoAim::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct AutoAim::Params));
}