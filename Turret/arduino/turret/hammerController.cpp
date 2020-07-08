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
//  interrupt driven state machine, implemented at the end of this file
//  after the class method implementation
//

#include "Arduino.h"
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "pins.h"
#include "Wire.h"

#include "sbus.h"
#include "autoaim.h"
#include "autofire.h"
#include "DMASerial.h"

#include "turretController.h"
#include "telemetryController.h"
#include "radioController.h"

#include "hammerController.h"


//  ====================================================================
//
//  External references
//
//  ====================================================================

//  ====================================================================
//
//  File constants 
//
//  ====================================================================

static const int16_t k_invalidHammerAngleRead = -1;
static const int16_t k_invalidHammerThrowPressureRead = -1;
static const int16_t k_invalidHammerRetractPressureRead = -1;

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
volatile static bool s_swingComplete;

//  These are file static rather than member variables because they are 
//  being accesses from an interrupt service routing

volatile static int16_t s_hammerAngleCurrent = k_invalidHammerAngleRead;
volatile static int32_t s_hammerSpeedCurrent = 0;

volatile static int16_t s_hammerThrowPressureCurrent = k_invalidHammerThrowPressureRead;
volatile static int16_t s_hammerRetractPressureCurrent = k_invalidHammerRetractPressureRead;

volatile static int16_t s_hammerThrowAngle = 0;

//  ====================================================================
//
//  Forward references to internal (private) methods
//
//  ====================================================================

void startFullCycleStateMachine();
void startRetractOnlyStateMachine();
void startSensorReadStateMachine();

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
    m_lastUpdateTime = micros();
    setState(EInit);
}

void HammerController::Update()
{
    m_lastUpdateTime = micros();

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

                if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && Radio.IsNominal())
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
                if (!Radio.IsNominal())
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
                if (!Radio.IsNominal())
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
                    Telem.LogMessage("Swing Complete");
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

    updateSpeed();

    Telem.LogMessage(String("Hammer angle = ") + s_hammerAngleCurrent);
}

void HammerController::Safe()
{
    setState(ESafe);
}

bool HammerController::ReadyToSwing()
{
    return m_state == EReady;
}

//
//  IMPORTANT: TriggerSwing, TriggerSelfRightSwing and Retract setup a secondary state
//  machine that is driven using timmer interrupts.  The normal
//  update loop is too variable and often not responsive enought.
//

void HammerController::TriggerSwing()
{
    Telem.LogMessage("FIRE");
    setState(EThrow);
    setState(EFullCycleInterruptMode);
}

//
//  Swing the hammer with fixed parameters to try and flip
//  us back over to right side up!
//

void HammerController::TriggerSelfRightSwing()
{
    setState(EThrowSelfRight);
    setState(EFullCycleInterruptMode);
}

//
//  Just incase the normal cycle retract didn't get the job done,
//  try to just fully retract the hammer
//

void HammerController::TriggerRetract()
{
    setState(ERetract);
    setState(ERetractOnlyInterruptMode);
}

int32_t HammerController::GetHammerSpeed()
{
    return s_hammerSpeedCurrent;
}

int16_t HammerController::GetHammerAngle()
{
    return s_hammerAngleCurrent;
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

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void HammerController::updateSpeed()
{
    //  Not calculating right Now

    s_hammerSpeedCurrent = 0;
}

void HammerController::setState(controllerState p_state)
{
    if (m_state == p_state)
    {
        return;
    }

    Telem.LogMessage(String("HammerController::setState = ") + p_state);

    //  exit state transition

    switch (m_state)
    {
        case EInit:
        {
        }
        break;

        case EFullCycleInterruptMode:
        {
            Turret.FlamePulseStop();
        }
    }

    m_state = p_state;
    m_stateStartTime = m_lastUpdateTime;

    //  enter state transition

    switch (m_state)
    {
        case EInit:
        {
            m_pAutoFire = new AutoFire();
            init();
        }
        break;

        case EThrow:
        {
            int32_t requestedIntensity = getHammerIntensity();
            m_throwPressureAngle = k_throwIntensityToAngle[requestedIntensity];
        }
        break;
        
        case EThrowSelfRight:
        {
            m_throwPressureAngle = m_params.selfRightIntensity;
        }
        break;

        case EFullCycleInterruptMode:
        {
            if (Radio.IsFlamePulseEnabled())
            {
                Turret.FlamePulseStart();
            }

            //  Write information into shared variables

            s_hammerThrowAngle = m_throwPressureAngle;
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
            //Telem.SendSwingTelem();
        }
        break;
    }
}

void HammerController::init()
{
    m_pAutoFire->Init();

    startSensorReadStateMachine();

    digitalWrite(THROW_PRESSURE_VALVE_DO, LOW); 
    pinMode(THROW_PRESSURE_VALVE_DO, OUTPUT);

    digitalWrite(THROW_VENT_VALVE_DO, LOW); 
    pinMode(THROW_VENT_VALVE_DO, OUTPUT);

    digitalWrite(RETRACT_PRESSURE_VALVE_DO, LOW); 
    pinMode(RETRACT_PRESSURE_VALVE_DO, OUTPUT);

    digitalWrite(RETRACT_VENT_VALVE_DO, LOW); 
    pinMode(RETRACT_VENT_VALVE_DO, OUTPUT);
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

static const uint8_t k_valveCloseDt = 10;                          //  10 microseconds
static const uint8_t k_valveOpenDt = 10;                           //  10 microseconds

static const uint16_t k_maxThrowAngle = 210;                       //  210 degrees
static const uint32_t k_maxThrowPressureDt = 750000;               //  0.75 second
static const uint32_t k_maxThrowDt = 1000000;                      //  1.00 second

static const uint16_t k_minRetractAngle = 5;                       //    5 degrees
static const uint32_t k_maxRetractDt = 1000000;                    //  1.0 seconds

static const uint32_t k_ATMega2560_ClockFrequency = F_CPU;         //  ATMega2560 is 16MHz
static const uint32_t k_subStateMachineUpdateFrequency = 50000;    //  50kHz or update every 20 microseconds
                                                                   //  at 100kHz, serail communication to Robotek stoped working

static const uint16_t k_telmSamplesMax = 500;
static const int32_t k_angleReadSwitchToPressureCount = 10;        //  10 Angle Reads for each pressure read

static const int16_t k_minValidHammerAngleDifferential = 120;
static const int16_t k_maxValidHammerAngleDifferential = 920;
static const int16_t k_minExpectedHammerAngleDifferential = 350;
static const int16_t k_maxExpectedHammerAngleDifferential = 920;

static const int16_t k_minValidThrowPressureDifferential = 102;
static const int16_t k_maxValidThrowPressureDifferential = 920;
static const int16_t k_minExpectedThrowPressureDifferential = 350;
static const int16_t k_maxExpectedThrowPressureDifferential = 920;

static const int16_t k_minValidRetractPressureDifferential = 102;
static const int16_t k_maxValidRetractPressureDifferential = 920;
static const int16_t k_minExpectedRetractPressureDifferential = 350;
static const int16_t k_maxExpectedRetractPressureDifferential = 920;

//  ====================================================================
//
//  File scope variables
//
//  ====================================================================

//  Telemetry info

volatile static uint32_t s_swingStartTime;
volatile static uint32_t s_swingStopTime;
volatile static uint32_t s_retractStartTime;
volatile static uint32_t s_retractStopTime;

volatile static uint16_t s_swingStartAngle;
volatile static uint16_t s_swingStopAngle;
volatile static uint16_t s_retractStartAngle;
volatile static uint16_t s_retractStopAngle;

volatile static uint16_t s_swingAngleSamples[k_telmSamplesMax];
volatile static uint8_t s_swingThrowPressureSamples[k_telmSamplesMax];
volatile static uint8_t s_swingRetractPressureSamples[k_telmSamplesMax];

volatile static uint16_t s_swingTelemSamplesCount = 0;

//  State machine states

enum hammerSubState
{
    EThrowSetup,
    EThrowPressurize,
    EThrowExpand,
    ERetractSetup,
    ERetractPressurize,
    ERetractComplete
};

enum sensorReadState
{
    EReadHammerAngle,
    EReadThrowPressure,
    EReadRetractPressure
};

static hammerSubState s_hammerSubState;
static uint32_t s_hammerSubStateStart = 0;
static uint32_t s_hammerSubStateDt = 0;

static sensorReadState s_sensorReadState;
static int32_t s_sensorAngleReadCount = 0;

static uint32_t s_swingTimeStart = 0;

//  ====================================================================
//
//  Macros
//
//  ====================================================================

#define SELECT_THROW_PRESSURE_READ { s_sensorReadState = EReadThrowPressure; ADMUX = (0x01 << 6) | ((HAMMER_THROW_PRESSURE_AI - 54) & 0x07); }
#define SELECT_RETRACT_PRESSURE_READ { s_sensorReadState = EReadRetractPressure; ADMUX = (0x01 << 6) | ((HAMMER_RETRACT_PRESSURE_AI - 54) & 0x07); }
#define SELECT_HAMMER_ANGLE_READ { s_sensorReadState = EReadHammerAngle; ADMUX = (0x01 << 6) | ((HAMMER_ANGLE_AI - 54) & 0x07); }

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

#define OPEN_THROW_PRESSURE { PORTH |= 1 << PORTH3; }
#define CLOSE_THROW_PRESSURE { PORTH &= ~(1 << PORTH3); }

#define OPEN_THROW_VENT { PORTH &= ~(1 << PORTH4); }
#define CLOSE_THROW_VENT { PORTH |= 1 << PORTH4; }

#define OPEN_RETRACT_PRESSURE { PORTH |= 1 << PORTH5; }
#define CLOSE_RETRACT_PRESSURE { PORTH &= ~(1 << PORTH5); } 

#define OPEN_RETRACT_VENT { PORTH &= ~(1 << PORTH6); }
#define CLOSE_RETRACT_VENT { PORTH |= 1 << PORTH6; }

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
    s_swingComplete = false;
    s_swingTelemSamplesCount = 0;

    s_swingTimeStart = micros();
    s_swingStartAngle = s_hammerAngleCurrent;

    //  Enter start state

    s_hammerSubState = EThrowSetup;
    s_hammerSubStateStart = s_swingTimeStart;

    //  close throw vent
    digitalWrite(THROW_VENT_VALVE_DO, HIGH); 

    //  close retract vent
    digitalWrite(RETRACT_VENT_VALVE_DO, HIGH); 
 
    clearPWMForDigitalWrites();
    startTimers();
}

void startRetractOnlyStateMachine()
{
    s_swingComplete = false;
    s_swingTelemSamplesCount = 0;

    s_retractStartTime = micros();
    s_retractStartAngle = s_hammerAngleCurrent;

    //  Enter start state

    s_hammerSubState = ERetractSetup;
    s_hammerSubStateStart = s_swingTimeStart;

    //  open throw vent
    digitalWrite(THROW_VENT_VALVE_DO, LOW); 
    
    //  close retract vent
    digitalWrite(RETRACT_VENT_VALVE_DO, HIGH); 
    
    clearPWMForDigitalWrites();
    startTimers();
}

void startSensorReadStateMachine()
{
    pinMode(HAMMER_ANGLE_AI, INPUT);
    pinMode(HAMMER_THROW_PRESSURE_AI, INPUT);
    pinMode(HAMMER_THROW_PRESSURE_AI, INPUT);

    //  Enable ADC, ADC interruupts and set the prescaler to 64, not the default 128

    ADCSRA = (0x01 << ADEN) | (0x01 << ADIE) | (0x01 << ADPS2) | (0x01 << ADPS1) | (0x01 << ADPS0);
    ADCSRB = 0x00;

    //  Start sesnor reads

    SELECT_THROW_PRESSURE_READ;
    ADCSRA |= 0x01 << ADSC;
}

void swingComplete()
{
    stopTimers();

    Telem.SendSwingTelem(
        s_swingTelemSamplesCount, 
        s_swingAngleSamples,
        s_swingThrowPressureSamples,
        s_swingRetractPressureSamples,
        s_telemetryFrequency,
        s_swingStartTime, s_swingStartAngle,
        s_swingStopTime, s_swingStopAngle,
        s_retractStartTime, s_retractStartAngle,
        s_retractStopTime, s_retractStopAngle);
}

//  ====================================================================
//
//  Interrupt Service Routines
//
//  ====================================================================

//  Interrupt Service Routine to do read hammer angle and pressures

ISR(ADC_vect)
{
    //  Get ADC results

    int16_t differential = ADCL | (ADCH << 8);
    
    //  Need to keep track of which analog pin we are reading

    switch (s_sensorReadState)
    {
        case EReadThrowPressure:
        {
            // calculate throw pressure

            s_hammerThrowPressureCurrent = k_invalidHammerThrowPressureRead;

            if (differential >= k_minExpectedThrowPressureDifferential && differential <= k_maxExpectedThrowPressureDifferential)
            {
                //  ((differential * 11) / 25) = conversion of differential range into anggle range

                s_hammerThrowPressureCurrent = ((((1024 - differential) - k_minValidThrowPressureDifferential) * 11) / 25);
            }
            else if (differential >= k_minValidThrowPressureDifferential && differential <= k_maxValidThrowPressureDifferential)
            {
                //  We believe the Analog read was good, but we don't expect the hammer to be in this
                //  position.  It likely just retracted too far.  Set to 0 degrees

                s_hammerThrowPressureCurrent = 0;
            }

            //  Set up next analog read

            SELECT_RETRACT_PRESSURE_READ;
        }
        break;

        case EReadRetractPressure:
        {
            // calculate throw pressure

            s_hammerRetractPressureCurrent = k_invalidHammerRetractPressureRead;

            if (differential >= k_minExpectedRetractPressureDifferential && differential <= k_maxExpectedRetractPressureDifferential)
            {
                //  ((differential * 11) / 25) = conversion of differential range into anggle range

                s_hammerRetractPressureCurrent = ((((1024 - differential) - k_minValidRetractPressureDifferential) * 11) / 25);
            }
            else if (differential >= k_minValidRetractPressureDifferential && differential <= k_maxValidRetractPressureDifferential)
            {
                //  We believe the Analog read was good, but we don't expect the hammer to be in this
                //  position.  It likely just retracted too far.  Set to 0 degrees

                s_hammerRetractPressureCurrent = 0;
            }

            //  Set up next analog read

            SELECT_HAMMER_ANGLE_READ;
        }
        break;
        
        case EReadHammerAngle:
        {
            // calculate Hammer Angle

            s_hammerAngleCurrent = k_invalidHammerAngleRead;

            if (differential >= k_minExpectedHammerAngleDifferential && differential <= k_maxExpectedHammerAngleDifferential)
            {
                //  ((differential * 11) / 25) = conversion of differential range into anggle range

                s_hammerAngleCurrent = ((((1024 - differential) - k_minValidHammerAngleDifferential) * 11) / 25);
            }
            else if (differential >= k_minValidHammerAngleDifferential && differential <= k_maxValidHammerAngleDifferential)
            {
                //  We believe the Analog read was good, but we don't expect the hammer to be in this
                //  position.  It likely just retracted too far.  Set to 0 degrees

                s_hammerAngleCurrent = 0;
            }

            //  Set up next analog read

            s_sensorAngleReadCount++;

            if (s_sensorAngleReadCount >= k_angleReadSwitchToPressureCount)
            {
                s_sensorAngleReadCount = 0;                
                SELECT_THROW_PRESSURE_READ;
            }
        }
        break;
    }

    //  Start next sesnor reads

    ADCSRA |= 0x01 << ADSC;
}

//  Interrupt Service Routine to collect telemetry, at defined frequency, durring a hammer throw and retract

ISR(TIMER4_COMPA_vect)
{
    if (s_hammerSubState != ERetractComplete)
    {
        s_swingAngleSamples[s_swingTelemSamplesCount] = s_hammerAngleCurrent;
        s_swingThrowPressureSamples[s_swingTelemSamplesCount] = s_hammerThrowPressureCurrent;
        s_swingRetractPressureSamples[s_swingTelemSamplesCount] = s_hammerRetractPressureCurrent;

        s_swingTelemSamplesCount++;
    }
}

//  Interrupt Service Routing triggerd at a defined frequency to check for and execute state transitions.

ISR(TIMER5_COMPA_vect)
{
    uint32_t now = micros();    
    s_hammerSubStateDt = now - s_hammerSubStateStart;

    hammerSubState desiredState = s_hammerSubState;

    switch (s_hammerSubState)
    {
        case EThrowSetup:
        {
            if (s_hammerSubStateDt >= k_valveCloseDt)
            {
                //  Go to EThrowPressurize state
                desiredState = EThrowPressurize;
                OPEN_THROW_PRESSURE;
            }
        }
        break;

        case EThrowPressurize:
        {
            if (s_hammerAngleCurrent > s_hammerThrowAngle || s_hammerSubStateDt > k_maxThrowPressureDt)
            {
                s_swingStopTime = now;
                s_swingStopAngle = s_hammerAngleCurrent;

                //  Go to EThrowExpand state

                desiredState = EThrowExpand;
                CLOSE_THROW_PRESSURE;
            }
        }
        break;

        case EThrowExpand:
        {
            if (s_hammerAngleCurrent >= k_maxThrowAngle || s_hammerSubStateDt > k_maxThrowDt)
            {
                s_retractStartTime = now;
                s_retractStartAngle = s_hammerAngleCurrent;

                // Go to ERetractSetup state

                desiredState = ERetractSetup;
                OPEN_THROW_VENT;
            }
        }
        break;

        case ERetractSetup:
        {
            if (s_hammerSubStateDt >= k_valveCloseDt)
            {
                //  Go to ERetractPressurize state
                desiredState = ERetractPressurize;
                OPEN_RETRACT_PRESSURE;
            }
        }
        break;

        case ERetractPressurize:
        {
            if ((s_hammerAngleCurrent >= 0 && s_hammerAngleCurrent <= k_minRetractAngle) || s_hammerSubStateDt > k_maxRetractDt)
            {
                s_retractStopTime = now;
                s_retractStopAngle = s_hammerAngleCurrent;

                //  Go to ERetractComplete state

                desiredState = ERetractComplete;

                CLOSE_RETRACT_PRESSURE;
                OPEN_RETRACT_VENT;

                s_swingComplete = true;
            }
        }
        break;
    }

    if (desiredState != s_hammerSubState)
    {
        s_hammerSubState = desiredState;
        s_hammerSubStateStart = now;
    }
}
