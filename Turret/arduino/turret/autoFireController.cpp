//  ====================================================================
//
//  Auto Fire
//
//  The Auto Fire object is owned by the Turret Controller
//  and has only 1 job...
//
//  Figure out when target is in the killzone and signal a swing
//  should be performed.  Turret Controller will poll for this
//  state and tringger swing.
//
//  ====================================================================


#include <Arduino.h>

#include "turretController.h"
#include "sbus.h"
#include "telemetryController.h"
#include "fixedpoint.h"
#include "radioController.h"
#include "targetTrackingController.h"
#include "autoFireController.h"

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

static struct AutoFireController::Params EEMEM s_savedParams = 
{
    .xtol = 200,   // currently unused, front of box is depth
    .ytol = 200,
    .maxOmegaZ = 1787,   // rad/s * 2048 = 50 deg/sec
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

void AutoFireController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void AutoFireController::Update() 
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
                else if (Radio.IsAutoFireEnabled())
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
                else if (!Radio.IsAutoFireEnabled())
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
                else if (!Radio.IsAutoFireEnabled())
                {
                    setState(EDisabled);
                }
                else if (!TargetTracking.IsTrackingValidTarget())
                {
                    setState(ENoTarget);
                }
                else if (TargetTracking.WillHitTrackedTarget())
                {
                    setState(ESwingAtTarget);
                }
            }
            break;

            case ESwingAtTarget:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!Radio.IsAutoFireEnabled())
                {
                    setState(EDisabled);
                }
                else if (!TargetTracking.IsTrackingValidTarget())
                {
                    setState(ENoTarget);
                }
                else if (!TargetTracking.WillHitTrackedTarget())
                {
                    setState(ETrackingTarget);
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

bool AutoFireController::ShouldSwingAtTarget()
{
    return m_state == ESwingAtTarget;
}

void AutoFireController::SetParams(int16_t p_xtol, int16_t p_ytol, int16_t p_maxOmegaZ)
{
    m_params.xtol = p_xtol;
    m_params.ytol = p_ytol;
    m_params.maxOmegaZ = p_maxOmegaZ;

    saveParams();
}

void AutoFireController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct AutoFireController::Params));
}

void AutoFireController::SendTelem()
{
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================


void AutoFireController::setState(autoFireState p_state)
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

void AutoFireController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct AutoFireController::Params));
}