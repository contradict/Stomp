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

    //  Pass update to our owned objects

    m_pAutoAim->Update();

    //  Update our state

    int32_t desiredAutoAimSpeed = m_pAutoAim->GetDesiredTurretSpeed();
    int32_t desiredSBusSpeed = Turret.GetDesiredManualTurretSpeed();

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
                    if (isManualTurretEnabled())
                    {
                        setState(EManualControl);
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
                else if (isManualTurretEnabled())
                {
                    setState(EManualControl);
                }
            }
            break;

            case EManualControl:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (!isManualTurretEnabled())
                {
                    setState(EDisabled);
                }
                else if (isAutoAimEnabled() && abs(desiredSBusSpeed) < m_params.ManualControlOverideSpeed)
                {
                    setState(EAutoAim);
                }
            }
            break;

            case EAutoAim:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (!isManualTurretEnabled())
                {
                    setState(EDisabled);
                }
                else if (!isAutoAimEnabled())
                {
                    setState(EManualControl);
                }
                else if (abs(desiredSBusSpeed) >= m_params.ManualControlOverideSpeed)
                {
                    setState(EManualControl);
                }
                else if (desiredSBusSpeed != 0)
                {
                    setState(EAutoAimWithManualAssist);
                }
            }
            break;

            case EAutoAimWithManualAssist:
            {
                if (!isRadioConnected())
                {
                    setState(ESafe);
                }
                else if (desiredSBusSpeed == 0)
                {
                    setState(EAutoAim);
                }
                else if (abs(desiredSBusSpeed) >= m_params.ManualControlOverideSpeed)
                {
                    setState(EManualControl);
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

    updateSpeed();
}

int32_t TurretRotationController::GetCurrentSpeed() 
{ 
    return m_currentSpeed; 
}

void TurretRotationController::SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain, uint32_t p_telemetry_interval)
{
    m_pAutoAim->SetParams(p_proportionalConstant, p_derivativeConstant, p_steer_max, p_gyro_gain, p_telemetry_interval);  
}

void TurretRotationController::SetParams(uint32_t p_manualControlOverideSpeed)
{
    m_params.ManualControlOverideSpeed = p_manualControlOverideSpeed;
    saveParams();
}

void TurretRotationController::RestoreParams()
{
    m_pAutoAim->RestoreParams();
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TurretRotationController::Params));
}

void TurretRotationController::SendTelem()
{
    m_pAutoAim->SendTelem();
    sendTurretRotationTelemetry(m_state, m_currentSpeed);
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TurretRotationController::updateSpeed()
{
    switch (m_state)
    {
        case EManualControl:
        {
            //  In manual control, just take the request from the radio and 
            //  make that our speed

            setSpeed(Turret.GetDesiredManualTurretSpeed());
        }
        break;

        case EAutoAim:
        {
            //  In AutoAim, just take the request from targeting and
            //  make that our speed

            setSpeed(m_pAutoAim->GetDesiredTurretSpeed());
        }
        break;

        case EAutoAimWithManualAssist:
        {
            //  Here, the auto aim is tracking, but the human is trying to
            //  do minimal adjustments.  So, keep auto aim and just add on
            //  the radio requested speed

            setSpeed(Turret.GetDesiredManualTurretSpeed() + m_pAutoAim->GetDesiredTurretSpeed());
        }
        break;

        default:
        {
            if (m_currentSpeed != 0)
            {
                setSpeed(0);
            }
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
            m_pAutoAim = new AutoAim();
            initAllControllers();
        }
        break;

        case ESafe:
        {
            setSpeed(0);
        }
        break;
    }
}

void TurretRotationController::initAllControllers()
{
    m_pAutoAim->Init();
    initRoboTeq();
}

void TurretRotationController::initRoboTeq()
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

void TurretRotationController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretRotationController::Params));
}