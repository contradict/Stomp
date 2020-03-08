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
    if (m_state != ETrackingTarget)
    {
        m_desiredTurretSpeed = 0;
        return;
    }

    //  We should not be in ETrackingTarget, without a valid target,
    //  but, lets just be sure

    uint32_t now = micros();

    if (!m_pTarget->valid(now))
    {
        return;
    }    

    int32_t theta = m_pTarget->angle();
    int32_t vtheta = m_pTarget->vtheta();

    m_desiredTurretSpeed = m_params.steer_p * (0 - theta) / 16384L;
    m_desiredTurretSpeed += -m_params.steer_d * vtheta / 16384L;

    /*
     int16_t omegaZ = 0;
    if(getOmegaZ(&omegaZ)) 
    {
        m_desiredTurretSpeed += -m_params.gyro_gain * omegaZ / 1024;
    }

   int32_t tracked_r = integer_sqrt(
        (m_pTarget->x / 4L) * (m_pTarget->x / 4L) +
        (m_pTarget->y / 4L) * (m_pTarget->y / 4L));
        
    int32_t tracked_vr = integer_sqrt(
        (m_pTarget->vx / 4L) * (m_pTarget->vx / 4L)+
        (m_pTarget->vy / 4L) * (m_pTarget->vy / 4L));

    bias  = params.drive_p * ((int32_t)depth*4L - tracked_r)/16384L;
    bias += -params.drive_d * tracked_vr * 4 / 16384L;   
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