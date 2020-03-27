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
    .telemetryFrequency = 100,
};

static uint16_t s_telemetryFrequency;

volatile static uint16_t s_throwPressureAngle;
volatile static bool s_swingComplete;

//  ====================================================================
//
//  Forward references to internal (private) methods
//
//  ====================================================================

void startFullCycleStateMachine();
void startRetractOnlyStateMachine();
void swingComplete();

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

            case EFullCycleInterruptMode:
            case ERetractOnlyInterruptMode:
            {
                if (s_swingComplete)
                {
                    setState(ESwingComplete);
                }
            }
            break;

            case ESwingComplete:
            {
                setState(EReady);
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
        }
        break;
        
        case EThrowSelfRight:
        {
            resetTelem();

            m_throwStartTime = m_stateStartTime;
            m_throwPressureAngle = k_throwPressureAngleSelfRight;
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
            //  Write information into shared variables

            s_throwPressureAngle = m_throwPressureAngle;
            s_telemetryFrequency = m_params.telemetryFrequency;

            startFullCycleStateMachine();
        }
        break;

        case ERetractOnlyInterruptMode:
        {
            //  Write information into shared variables
            //  No need for s_throwPressureAngle becasue we are only retracting

            s_telemetryFrequency = m_params.telemetryFrequency;

            startRetractOnlyStateMachine();
        }
        break;

        case ESwingComplete:
        {
            swingComplete();
            //sendSwingTelem();
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

//  ====================================================================
//
//  constants
//
//  ====================================================================

const uint8_t k_valveCloseDt = 10;                          //  10 microseconds
const uint8_t k_valveOpenDt = 10;                           //  10 microseconds

const uint16_t k_maxThrowAngle = 210;                       //  210 degrees
const uint16_t k_maxThrowDt = 500000;                       //  0.5 seconds

const uint16_t k_minRetractAngle = 0;                       //    0 degrees
const uint16_t k_maxRetractDt = 2000000;                    //  2.0 seconds

const uint32_t k_ATMega2560_ClockFrequency = F_CPU;         //  ATMega2560 is 16MHz
const uint32_t k_subStateMachineUpdateFrequency = 100000;   //  100kHz or update every 10 microseconds

const uint16_t k_telmSamplesMax = 500;

//  ====================================================================
//
//  File scope variables
//
//  ====================================================================

struct swingTelm
{
    int16_t angle;
    int16_t throwPressure;
    int16_t retractPressure;
    int8_t state;
    int8_t valveState;
};

static swingTelm s_swingTelmSamples[k_telmSamplesMax];
static uint16_t s_swingTelemSamplesCount = 0;

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
static uint32_t s_hammerSubStateStart = 0;
static uint32_t s_hammerSubStateDt = 0;

static uint8_t s_valveState = 0x00;

static uint32_t s_swingTimeStart = 0;

static int32_t s_hammerAngleCurrent = 0;
static int32_t s_angularVelocity = 0;

//  ====================================================================
//
//  Macros
//
//  ====================================================================

//  Keep track of the state of each valve for telemerty reporting.
//  
//  0 in bit position means valve is CLOSED
//  1 in bit position means valve is OPEN
//

#define TP_VALVE_BIT 0
#define TV_VALVE_BIT 1
#define RP_VALVE_BIT 2
#define RV_VALVE_BIT 3

#define UPDATE_VALVE_STATE_TP_OPEN { s_valveState |= 1 << TP_VALVE_BIT; }
#define UPDATE_VALVE_STATE_TP_CLOSED { s_valveState &= ~(1 << TP_VALVE_BIT); }
#define UPDATE_VALVE_STATE_TV_OPEN { s_valveState |= 1 << TV_VALVE_BIT; }
#define UPDATE_VALVE_STATE_TV_CLOSED { s_valveState &= 1 << ~(TV_VALVE_BIT); }
#define UPDATE_VALVE_STATE_RP_OPEN { s_valveState |= 1 << RP_VALVE_BIT; }
#define UPDATE_VALVE_STATE_RP_CLOSED { s_valveState &= ~(1 << RP_VALVE_BIT); }
#define UPDATE_VALVE_STATE_RV_OPEN { s_valveState |= 1 << RV_VALVE_BIT; }
#define UPDATE_VALVE_STATE_RV_CLOSED { s_valveState &= 1 << ~(RV_VALVE_BIT); }

//
//  IMPORTANT: These MACROs are optomized for running from ISR (not for general use)
//  Have insured that PWM is disconnected from these pins and interrupts are disabled
//
//  Also there is no abstraction between logical pin and ATMega2560 ports so this only
//  works on our selected processor - the Mega2560
//
//  Pin assignemenst are as follows:
//
//                      normal          port
//  valve               state   pin     bit     PWM
//  ------------------- ------- ------- ------- ------
//  Throw Pressure      NC      6       H:3     OC4A
//  Throw Vent          NO      7       H:4     OC4B
//  Retract Pressure    NC      8       H:5     OC4C
//  Retract Vent        NO      9       H:6     OC2B
//

#define OPEN_THROW_PRESSURE { PORTH |= 1 << PORTH3; UPDATE_VALVE_STATE_TP_OPEN }
#define CLOSE_THROW_PRESSURE { PORTH &= ~(1 << PORTH3); UPDATE_VALVE_STATE_TP_CLOSED }

#define OPEN_THROW_VENT { PORTH &= ~(1 << PORTH4); UPDATE_VALVE_STATE_TV_OPEN }
#define CLOSE_THROW_VENT { PORTH |= 1 << PORTH4; UPDATE_VALVE_STATE_TV_CLOSED }

#define OPEN_RETRACT_PRESSURE { PORTH |= 1 << PORTH5; UPDATE_VALVE_STATE_RP_OPEN }
#define CLOSE_RETRACT_PRESSURE { PORTH &= ~(1 << PORTH5); UPDATE_VALVE_STATE_RP_CLOSED} 

#define OPEN_RETRACT_VENT { PORTH &= ~(1 << PORTH6); UPDATE_VALVE_STATE_RV_OPEN }
#define CLOSE_RETRACT_VENT { PORTH |= 1 << PORTH6; UPDATE_VALVE_STATE_RV_CLOSED }

#define READ_HAMMER_ANGLE (0)
#define READ_THROW_PRESSURE (0)
#define READ_RETRACT_PRESSURE (0)

//  ====================================================================
//
//  Methods
//
//  ====================================================================

void startTimers()
{
    noInterrupts();

    //  Setup timer 4A to interrupt at s_telemetryFrequency

    uint32_t timer4Count = (k_ATMega2560_ClockFrequency / s_telemetryFrequency);

    TCCR4A = 0;                             //  Clear control register A
    TCCR4B = 0;                             //  Clear control register B
    TCCR4B |= 1 << WGM42;                   //  Set Counter4A to CTC mode

    TIMSK4 |= 1 << OCIE4A;                  //  Turn on interrupt bit in mask
    OCR4A = timer4Count;                    //  Initialize the count to match
    TCNT5 = 0x0000;                         //  Clear the counter

    //  Setup timer 5A to interrupt at k_subStateMachineUpdateFrequency

    uint32_t timer5Count = (k_ATMega2560_ClockFrequency / k_subStateMachineUpdateFrequency);

    TCCR5A = 0;                             //  Clear control register A
    TCCR5B = 0;                             //  Clear control register B
    TCCR5B |= 1 << WGM52;                   //  Set Counter5A to CTC mode

    TIMSK5 |= 1 << OCIE5A;                  //  Turn on interrupt bit in mask
    OCR5A = timer5Count;                    //  Initialize the count to match
    TCNT5 = 0x0000;                         //  Clear the counter

    //  Start both timers and enable interrupts

    TCCR4B |= 1 << CS40;                    //  Enable timmer, no prescaller
    TCCR5B |= 1 << CS50;                    //  Enable timmer, no prescaller

    interrupts();
}

void stopTimers()
{
    noInterrupts();

    //  Turn off Timer 4

    TCCR4A = 0;                             //  Clear control register A
    TCCR4B = 0;                             //  Clear control register B
    TIMSK4 &= ~(1 << OCIE4A);               //  Turn off interrupt bit in mask

    //  Turn off Timer 5

    TCCR5A = 0;                             //  Clear control register A
    TCCR5B = 0;                             //  Clear control register B
    TIMSK5 &= ~(1 << OCIE5A);               //  Turn off interrupt bit in mask

    interrupts();
}

//  Ensure that PWM functionality is NOT enabled on pins we are using
//  to control valves.  This normally happens in the digitalWrite function
//  but the ISR here are using optimized digialWrite macros (above)

void clearPWMForDigitalWrites()
{
    TCCR2B &= ~(1 << COM2B0 | 1 << COM2B1);

    //  Timer 4 is one we are using and managing TCCR4A & TCCR4B and are cleared
    //  in the setTimers method, but just for clearity's sake this method is 
    //  going to clear PWM on each pin used.

    TCCR4A &= ~(1 << COM4A0 | 1 << COM4A1);
    TCCR4B &= ~(1 << COM4B0 | 1 << COM4B1);
    TCCR4C &= ~(1 << COM4C0 | 1 << COM4B1);
}

void startFullCycleStateMachine()
{
    uint32_t now = micros();

    s_swingTelemSamplesCount = 0;
    s_valveState = 1 << TP_VALVE_BIT | 1 << RP_VALVE_BIT;

    s_swingTimeStart = now;

    //  Enter start state

    s_hammerSubState = EThrowSetup;
    s_hammerSubStateStart = now;

    //  close throw vent
    digitalWrite(7, HIGH); 
    UPDATE_VALVE_STATE_TV_CLOSED;

    //  close retract vent
    digitalWrite(9, HIGH); 
    UPDATE_VALVE_STATE_RV_CLOSED;

    clearPWMForDigitalWrites();
    startTimers();
}

void startRetractOnlyStateMachine()
{
    uint32_t now = micros();

    s_swingTelemSamplesCount = 0;
    s_valveState = 1 << TP_VALVE_BIT | 1 << RP_VALVE_BIT;

    s_swingTimeStart = now;

    //  Enter start state

    s_hammerSubState = ERetractSetup;
    s_hammerSubStateStart = now;

    //  open throw vent
    digitalWrite(7, LOW); 
    UPDATE_VALVE_STATE_TV_OPEN;
    
    //  close retract vent
    digitalWrite(9, HIGH); 
    UPDATE_VALVE_STATE_RV_CLOSED;
    
    clearPWMForDigitalWrites();
    startTimers();
}

void swingComplete()
{
    stopTimers();
}

//  ====================================================================
//
//  Interrupt Service Routines
//
//  ====================================================================

//  Interrupt Service Routine to collect telemetry, at defined frequency, durring a hammer throw and retract

ISR(TIMER4_COMPA_vect)
{
    if (s_hammerSubState != ERetractComplete)
    {
        s_swingTelmSamples[s_swingTelemSamplesCount].angle = READ_HAMMER_ANGLE;
        s_swingTelmSamples[s_swingTelemSamplesCount].throwPressure = READ_THROW_PRESSURE;
        s_swingTelmSamples[s_swingTelemSamplesCount].retractPressure = READ_RETRACT_PRESSURE;
        s_swingTelmSamples[s_swingTelemSamplesCount].state = s_hammerSubState;
        s_swingTelmSamples[s_swingTelemSamplesCount].valveState = s_valveState;
        s_swingTelemSamplesCount++;
    }
}

//  Interrupt Service Routing triggerd at a defined frequency to check for and execute state transitions.

ISR(TIMER5_COMPA_vect)
{
    uint32_t now = micros();
    
    s_hammerSubStateDt = now - s_hammerSubStateStart;
    s_hammerAngleCurrent = READ_HAMMER_ANGLE;

    switch (s_hammerSubState)
    {
        case EThrowSetup:
        {
            if (s_hammerSubStateDt >= k_valveCloseDt)
            {
                //  Go to EThrowPressurize state
                s_hammerSubState = EThrowPressurize;
                s_hammerSubStateStart = now;
                OPEN_THROW_PRESSURE;
            }
        }
        break;

        case EThrowPressurize:
        {
            if (s_hammerAngleCurrent > s_throwPressureAngle)
            {
                //  Go to EThrowExpand state
                s_hammerSubState = EThrowExpand;
                s_hammerSubStateStart = now;
                CLOSE_THROW_PRESSURE;
            }
        }
        break;

        case EThrowExpand:
        {
            if (s_hammerAngleCurrent >= k_maxThrowAngle || s_hammerSubStateDt > k_maxThrowDt)
            {
                // Go to ERetractSetup state
                s_hammerSubState = ERetractSetup;
                s_hammerSubStateStart = now;
                OPEN_THROW_VENT;
            }
        }
        break;

        case ERetractSetup:
        {
            if (s_hammerSubStateDt >= k_valveCloseDt)
            {
                //  Go to ERetractPressurize state
                s_hammerSubState = ERetractPressurize;
                s_hammerSubStateStart = now;
                OPEN_RETRACT_PRESSURE;
            }
        }
        break;

        case ERetractPressurize:
        {
            if (s_hammerAngleCurrent <= k_minRetractAngle || s_hammerSubStateDt > k_maxRetractDt)
            {
                //  Go to ERetractComplete state
                s_hammerSubState = ERetractComplete;
                s_hammerSubStateStart = now;
                CLOSE_RETRACT_PRESSURE;
                OPEN_RETRACT_VENT;
            }
        }
        break;
    }
}

