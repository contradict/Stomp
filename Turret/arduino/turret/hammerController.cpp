//
//  Hammer Controller
//
//  Throw (or swing) the hammer with pneumatic valve open for a period of
//  of time, then close that valve and the the throw continue.  Then retract
//  the hammer and get ready for another throw.
//
//  This is accomplished with 4 valves to control pneumatics: 
//
//      Throw Pressure, Throw Vent, Retract Pressure, Retract Vent
//
//  Due to the strict and short timing requirements, the valve control
//  and hammer angular velocity calculations are handled in an 
//  interrupt drive state machine, implemented at the end of this file
//  after the class method implementation
//

#include "Arduino.h"
#include <avr/interrupt.h>
#include <util/atomic.h>
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
    .selfRightIntensity = 75,
    .telemetryFrequency = 10000,
};

static uint32_t s_throwPressureDt;

//  ====================================================================
//
//  Forward references to internal (private) methods
//
//  ====================================================================

void startFullCycleStateMachine();
void startRetractOnlyStateMachine();

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
                        setState(EReady);
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
                else if (isWeaponEnabled())
                {
                    setState(EReady);
                }
            }
            break;

            case EReady:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (!isWeaponEnabled())
                {
                    setState(EDisabled);
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
//  IMPORTANT: Fire, FireSelfRight and Retract setup a secondary state
//  machine that is driven using timmer interrupts.  The normal
//  update loop is too variable and often not responsive enought.
//

void HammerController::Fire()
{
    setState(EThrow);
    setState(EFullCycleInterruptMode);
}

//
//  Swing the hammer with fixed parameters to try and flip
//  us back over to right side up!
//

void HammerController::FireSelfRight()
{
    setState(EThrowSelfRight);
    setState(EFullCycleInterruptMode);
}

//
//  Just incase the normal cycle retract didn't get the job done,
//  try to just fully retract the hammer
//

void HammerController::Retract()
{
    setState(ERetract);
    setState(ERetractOnlyInterruptMode);
}

void HammerController::SetAutoFireParameters(int16_t p_xtol, int16_t p_ytol, int16_t p_max_omegaz, uint32_t telemetry_interval)
{
    m_pAutoFire->SetParams(p_xtol, p_ytol, p_max_omegaz, telemetry_interval);
}

void HammerController::SetParams(uint32_t p_selfRightIntensity, uint32_t p_telemetryFrequency)
{
    m_params.selfRightIntensity = p_selfRightIntensity;
    m_params.telemetryFrequency = p_telemetryFrequency;

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

        case EThrow:
        {
            resetTelem();

            int32_t requestedIntensity = getHammerIntensity();

            m_throwStartTime = m_stateStartTime;
            m_throwPressureAngle = k_throwIntensityToAngle[requestedIntensity];
            m_throwPressureDt = k_throwIntensityToDt[requestedIntensity];
        }
        break;
        
        case EThrowSelfRight:
        {
            resetTelem();

            m_throwStartTime = m_stateStartTime;
            m_throwPressureAngle = k_throwPressureAngleSelfRight;
            m_throwPressureDt = m_params.selfRightIntensity;
        }
        break;

        case ERetract:
        {
            resetTelem();

            m_retractOnlyPhase = true;
        }
        break;

        case EFullCycleInterruptMode:
        {
            s_throwPressureDt = m_throwPressureDt;
            startFullCycleStateMachine();
        }
        break;

        case ERetractOnlyInterruptMode:
        {
            startRetractOnlyStateMachine();
        }
        break;
    }
}

void HammerController::initAllControllers()
{
    m_pAutoFire->Init();
}

void HammerController::resetTelem()
{
    m_retractOnlyPhase = false;
}

void HammerController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct HammerController::Params));
}

//  ====================================================================
//
//  Interrupt driven state machine implementation
//
//  ====================================================================

const uint8_t k_valveCloseDt = 10;
const uint8_t k_valveOpenDt = 10;

enum hammerSubState
{
    EThrowSetup,
    EThrowPressurize,
    EThrowExpand,
    ERetractSetup,
    ERetractPressurize,
    ERetractComplete
};

static hammerSubState s_hammerSubState;

static uint32_t s_cycleTimeStart = 0;
static uint32_t s_hammerSubStateStart = 0;
static uint32_t s_hammerSubStateDt = 0;

static int32_t s_angularVelocity = 0;

void startFullCycleStateMachine()
{
    s_cycleTimeStart = micros();
    s_hammerSubState = EThrowSetup;

    //  Easier to just do a delay(k_valveCloseDt) here than
    //  deal with interrupts for this.

    // closeThrowVent();
    delay(k_valveCloseDt);

    s_hammerSubState = EThrowPressurize;
    //  openThrowPressure()
}

void startRetractOnlyStateMachine()
{
    s_cycleTimeStart = micros();
    s_hammerSubState = ERetractSetup;

    //  Easier to just do a delay(k_valveCloseDt) here than
    //  deal with interrupts for this.

    //openThrowVent();
    //closeRetractVent();

    delay(max(k_valveCloseDt, k_valveOpenDt));

    s_hammerSubState = ERetractPressurize;
    // openRetractPressure();
}

//  Interrupt Service Routine to collect telemetry, at defined frequency, durring a hammer throw and retract

ISR(TIMER4_COMPA_vect)
{
    if (s_hammerSubState != ERetractComplete)
    {
    }
}

//  Interrupt Service Routing that triggers when we have waited the the desired amount of
//  time to leave the throwPressure valve open.
//  
//  Handles State Transitions:
//
//      * EThrowPressurize -> EThrowExpand state transition)
//

ISR(TIMER5_COMPA_vect)
{
    // closeThrowPressure();
    s_hammerSubState = EThrowExpand;
}

//  Interrupt Service Routing triggerd at a defined frequency to chech for state transitions.
//  
//  Handles State Transitions:
//
//      * EThrowExpand -> ERetractSetup
//      * ERetractSetup -> ERetractPressurize
//      * ERetractPressurize -> ERetractComplete
//

ISR(TIMER4_COMPB_vect)
{
    if (s_hammerSubState == EThrowExpand)
    {
        //  Wait until angle >= max angle || timeout

        // openThrowVent();
        s_hammerSubState = ERetractSetup;
    }
    else if (s_hammerSubState == ERetractSetup)
    {
        //  Wait until we have been in this start at lease k_valveCloseDt

        // openRetractPressure();
        s_hammerSubState = ERetractPressurize;
    }
    else if (s_hammerSubState == ERetractPressurize)
    {
        //  Wait until angle <= retracted angle || timeout

        //  openRetractVent();
        s_hammerSubState = ERetractComplete;
    }
}

