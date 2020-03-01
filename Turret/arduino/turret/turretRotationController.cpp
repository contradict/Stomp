//
//  Turret Rotation Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telem.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turretController.h"
#include "turretRotationController.h"

//  ====================================================================
//
//  External references
//
//  ====================================================================

//  Serial out pins defined in turret.ino-- check there to 
//  verify proper connectivity to motor controller

extern HardwareSerial& TurretRotationMotorSerial;

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static struct TurretRotationController::Params EEMEM s_savedParams = 
{
    .ManualControlOverideSpeed = 250,
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

void TurretRotationController::Init()
{
    m_state = EInvalid;
    setState(EInit);
}

void TurretRotationController::Update()
{
    uint32_t now = micros();
    int32_t desiredAutoAimSpeed = Turret.GetDesiredAutoAimSpeed();
    int32_t desiredSBusSpeed = Turret.GetDesiredSBusSpeed();

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
                    setState(EIdle);
                }
            }
            break;

            case EIdle:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (desiredSBusSpeed != 0)
                {
                    setState(EManualControl);
                }
                else if (desiredAutoAimSpeed != 0)
                {
                    setState(EAutoAim);
                }
            }
            break;

            case EManualControl:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (desiredSBusSpeed == 0)
                {
                    setState(EIdle);
                }
                else if (desiredAutoAimSpeed != 0 && abs(desiredSBusSpeed) < m_params.ManualControlOverideSpeed)
                {
                    setState(EAutoAimWithManualAssist);
                }
            }
            break;

            case EAutoAim:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (desiredAutoAimSpeed == 0)
                {
                    setState(EIdle);
                }
                else if (desiredSBusSpeed != 0)
                {
                    if (abs(desiredSBusSpeed) >= m_params.ManualControlOverideSpeed)
                    {
                        setState(EManualControl);
                    }
                    else
                    {
                        setState(EAutoAimWithManualAssist);
                    }
                }
            }
            break;

            case EAutoAimWithManualAssist:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (abs(desiredSBusSpeed) >= m_params.ManualControlOverideSpeed)
                {
                    setState(EManualControl);
                }
                else if (desiredSBusSpeed == 0)
                {
                    if (abs(desiredAutoAimSpeed) > 0)
                    {
                        setState(EAutoAim);
                    } 
                    else
                    {
                        setState(EIdle);
                    }
                }
            }
            break;

        }

        if (m_state == prevState)
        {
            break;
        }
    }

    //  Now that the state is stable, take action based on stable state

    updateSpeed();
}

void TurretRotationController::SetParams(uint32_t p_manualControlOverideSpeed)
{
    m_params.ManualControlOverideSpeed = p_manualControlOverideSpeed;
    saveParams();
}

void TurretRotationController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TurretRotationController::Params));
}

void TurretRotationController::SendTelem()
{
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TurretRotationController::initMotorController()
{
    TurretRotationMotorSerial.begin(115200);

    // set serial priority first
    TurretRotationMotorSerial.println("@00^CPRI 1 0");
    delay(5);

    // set RC priority second
    TurretRotationMotorSerial.println("@00^CPRI 2 1");
    delay(5);

    // turn off command echo
    TurretRotationMotorSerial.println("@00^ECHOF 1");
    delay(5);

    // set RS232 watchdog to 100 ms
    TurretRotationMotorSerial.println("@00^RWD 100");
}

void TurretRotationController::updateSpeed()
{
    switch (m_state)
    {
        case EManualControl:
        {
            //  In manual control, just take the request from the radio and 
            //  make that our peed

            setSpeed(desiredSBusTurretSpeed());
        }
        break;

        case EAutoAim:
        {
            //  In AutoAim, just take the request from targeting and
            //  make that our speed

            setSpeed(desiredAutoAimTurretSpeed());
        }
        break;

        case EAutoAimWithManualAssist:
        {
            //  Here, the auto aim is tracking, but the human is trying to
            //  do minimal adjustments.  So, keep auto aim and just add on
            //  the radio requested speed

            setSpeed(desiredSBusTurretSpeed() + desiredAutoAimTurretSpeed());
        }
        break;

        default:
        {
            setSpeed(0);
        }
        break;
    }
}

void TurretRotationController::setState(controllerState p_state)
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
            initMotorController();
        }
        break;
    }

    m_state = p_state;
    m_stateStartTime = micros();

    //  enter state transition

    switch (m_state)
    {
        case ESafe:
        {
            setSpeed(0);
        }
        break;
    }
}

void TurretRotationController::setSpeed(int32_t p_speed)
{
    m_currentSpeed = p_speed;
    m_currentSpeed = constrain(p_speed, k_minSpeed, k_maxSpeed);

    //  send "@nn!G mm" over software serial. mm is a command 
    //  value, -1000 to 1000. nn is node number in RoboCAN network.
    
    TurretRotationMotorSerial.print("@01!G ");
    TurretRotationMotorSerial.println(m_currentSpeed);

    TurretRotationMotorSerial.print("@02!G ");
    TurretRotationMotorSerial.println(m_currentSpeed);

    TurretRotationMotorSerial.print("@03!G ");
    TurretRotationMotorSerial.println(m_currentSpeed);

    TurretRotationMotorSerial.print("@04!G ");
    TurretRotationMotorSerial.println(m_currentSpeed);
}

void TurretRotationController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretRotationController::Params));
}