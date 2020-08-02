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

#include "sbus.h"
#include "telemetryController.h"
#include "targetTrackingController.h"
#include "fixedpoint.h"

#include "radioController.h"
#include "turretController.h"
#include "AutoAimController.h"


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

static struct AutoAimController::Params EEMEM s_savedParams = 
{
    .proportionalConstant = 2500,
    .integralConstant = 7500,
    .derivativeConstant = 5,
    .speedMax = 1000,
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

void AutoAimController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void AutoAimController::Update() 
{
    uint32_t previousUpdateTime = m_lastUpdateTime;

    m_lastUpdateTime = micros();
    m_latestDt = m_lastUpdateTime - previousUpdateTime;

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

                if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && Radio.IsNominal())
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
                if (!Radio.IsNominal())
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
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!isAutoAimEnabled())
                {
                    setState(EDisabled);
                }
                else if (TargetTracking.IsTrackingValidTarget())
                {
                    setState(ETrackingTarget);
                }
            }
            break;

            case ETrackingTarget:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!TargetTracking.IsTrackingValidTarget())
                {
                    setState(ENoTarget);
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

    updateDesiredTurretSpeed();
}

int32_t AutoAimController::GetDesiredTurretSpeed()
{
    return m_desiredTurretSpeed;
}

void AutoAimController::SetParams(int32_t p_proportionalConstant, int32_t p_integralConstant, int32_t p_derivativeConstant, int32_t p_speedMax)
{
    m_params.proportionalConstant = p_proportionalConstant;
    m_params.integralConstant = p_integralConstant;
    m_params.derivativeConstant = p_derivativeConstant;

    m_params.speedMax = p_speedMax;

    saveParams();
}

void AutoAimController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct AutoAimController::Params));
}

void AutoAimController::SendTelem()
{
    if (TargetTracking.IsTrackingValidTarget())
    {
        Telem.SendAutoAimTelemetry(m_state, m_desiredTurretSpeed, m_error, m_errorIntegral, m_errorDerivative);
    }
    else
    {
        Telem.SendAutoAimTelemetry(m_state, m_desiredTurretSpeed, 0, 0, 0);
    }
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void AutoAimController::updateDesiredTurretSpeed()
{
    if (m_state != ETrackingTarget)
    {
        m_desiredTurretSpeed = 0;
        return;
    }

    //
    //  IMPORTANT FIXED POINT NOTE, there are several places in this code where I am
    //  cheating.  Fixed point multiply needs to remove the multiplied scale from the
    //  product.  In several places I do not convert a number from regular integer
    //  into a sutibly scaled fixed point.  In this way I don't scale the number up
    //  to use in a multiply to then just remove the scale.
    //


    FP_32x20 dt = (TO_FP_32x20(1) / 1000000) * m_latestDt;

    //  We should not be in ETrackingTarget, without a valid target,
    //  but, lets just be sure

    if (!TargetTracking.IsTrackingValidTarget())
    {
        return;
    }    

    //  Using traditiona PID control to eliminate error (angle beteen forward and target)

    FP_32x20 prevError = m_error;

    m_error = TO_FP_32x20_FROM_FP_32x11(TargetTracking.GetTargetErrorAngle());
    m_errorDerivative = FP_32x20_POST_DIV((m_error - prevError) / dt);
    m_errorIntegral = m_errorIntegral + FP_32x20_POST_MUL(m_error * dt);

    //  multiply by the PID constants (not scaled) and then convert back to integers.  Anything lost
    //  in the conversion to int, will be insignificant in the range of desired speed (-speedMax, speedMax)

    int32_t proportionalComponent = FROM_FP_32x20_TO_INT(m_params.proportionalConstant * m_error);
    int32_t derivitiveComponent = FROM_FP_32x20_TO_INT(m_params.derivativeConstant * m_errorDerivative);
    int32_t intergalComponent = FROM_FP_32x20_TO_INT(m_params.integralConstant * m_errorIntegral);

    m_desiredTurretSpeed = proportionalComponent + derivitiveComponent + intergalComponent;
    m_desiredTurretSpeed = constrain(m_desiredTurretSpeed, -m_params.speedMax, m_params.speedMax);
}

void AutoAimController::setState(autoAimState p_state)
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

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
        }
        break;

        case ETrackingTarget:
        {
            m_error = TO_FP_32x20_FROM_FP_32x11(TargetTracking.GetTargetErrorAngle());

            m_errorDerivative = 0;
            m_errorIntegral = 0;
        }
        break;

        default:
        break;
    }
}

void AutoAimController::init()
{
    m_lastUpdateTime = 0;
    m_latestDt = 0;
}

void AutoAimController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct AutoAimController::Params));
}