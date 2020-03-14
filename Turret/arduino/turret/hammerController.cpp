//
//  Flame Thrower Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telem.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turretController.h"
#include "hammerController.h"

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

static struct HammerController::Params EEMEM s_savedParams = 
{
    .tmp = 0,
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

void HammerController::Init()
{
    m_state = EInvalid;
    setState(EInit);
}

void HammerController::Update()
{
    uint32_t now = micros();

    //  Pass update to our owned objects

    m_pAutoFire->Update();

    //  Update our state

    while(true)
    {
        controllerState prevState = m_state;

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
                    if (isWeaponEnabled())
                    {
                        setState(EReadyToFire);
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
            }
            break;
        }

        //  No more state changes, move on
        
        if (m_state == prevState)
        {
            break;
        }
    }

    //  Now that the state is stable, take action based on stable state

}

//
//  IMPORTANT: Fire and FireSelfRight completly take over processing
//  Neither of these functions reutrn until the hammer strike 
//  has completed.
//

void HammerController::Fire()
{
    uint32_t loopStartTime;
    uint32_t loopDt;
    uint32_t delayDt;

    setState(EFire);

    while (m_state != ESwingComplete && micros() - m_swingStartTime < k_swingTimeMaxDt)
    {
        uint32_t loopStartTime = micros();

        Update();

        uint32_t loopDt = micros() - loopStartTime;
        uint32_t delayDt = max(0, k_swingUpdateDt - loopDt);

        delay(k_swingUpdateDt);
    }
}

void HammerController::FireSelfRight()
{
    uint32_t loopStartTime;
    uint32_t loopDt;
    uint32_t delayDt;

    setState(EFireSelfRight);

    while (m_state != ESwingComplete && micros() - m_swingStartTime < k_swingTimeMaxDt)
    {
        uint32_t loopStartTime = micros();

        Update();

        uint32_t loopDt = micros() - loopStartTime;
        uint32_t delayDt = max(0, k_swingUpdateDt - loopDt);

        delay(k_swingUpdateDt);
    }
}

void HammerController::SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t telemetry_interval)
{
    m_pAutoFire->SetParams(p_xtol, p_ytol, p_max_omegaz, telemetry_interval);
}

void HammerController::SetParams(uint32_t p_manualControlOverideSpeed)
{
    saveParams();
}

void HammerController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct HammerController::Params));
}

void HammerController::SendTelem()
{
    /*
    bool sendSwingTelem(
        datapoints_collected,
        angle_data, 
        pressure_data, 
        data_collect_timestep,
        throw_close_timestep, 
        vent_open_timestep, 
        throw_close_angle,
        start_angle);
        */
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void HammerController::setState(controllerState p_state)
{
    if (m_state == p_state)
    {
        return;
    }

    //  exit state transition

    switch (m_state)
    {
        case EInit:
        {
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
            m_pAutoFire = new AutoFire();
            initAllControllers();
        }
        break;

        case ESafe:
        {
        }
        break;

        case EFire:
        {
            m_swingStartTime = m_stateStartTime;
        }
        break;
        
        case EFireSelfRight:
        {
            m_swingStartTime = m_stateStartTime;
        }
        break;

        break;
    }
}

void HammerController::initAllControllers()
{
    m_pAutoFire->Init();
}


void HammerController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct HammerController::Params));
}