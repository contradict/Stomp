//
//  Turret Rotation Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telemetryController.h"
#include "autoaim.h"
#include "DMASerial.h"

#include "turretController.h"
#include "radioController.h"
#include "turretRotationController.h"

//  ====================================================================
//
//  File constants 
//
//  ====================================================================

static const int16_t k_invalidTurretAngleRead = -1;

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static HardwareSerial& s_TurretRotationMotorSerial = Serial1;   // RX pin 19, TX pin 18

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
    m_lastUpdateTime = micros();
    setState(EInit);
}

void TurretRotationController::Update()
{
    m_lastUpdateTime = micros();

    //  Pass update to our owned objects

    m_pAutoAim->Update();

    //  Update our state

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

                if (m_lastUpdateTime - m_stateStartTime > k_safeStateMinDt && Radio.IsNominal())
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
                if (!Radio.IsNominal())
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
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!isManualTurretEnabled())
                {
                    setState(EDisabled);
                }
                else if (isAutoAimEnabled() && Turret.IsNominal() && abs(desiredSBusSpeed) < m_params.ManualControlOverideSpeed)
                {
                    setState(EAutoAim);
                }
            }
            break;

            case EAutoAim:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!isManualTurretEnabled())
                {
                    setState(EDisabled);
                }
                else if (!isAutoAimEnabled() || !Turret.IsNominal())
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
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (!isAutoAimEnabled() || !Turret.IsNominal())
                {
                    setState(EManualControl);
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

            default:
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
    updateAngle();
}

void TurretRotationController::Safe()
{
    setState(ESafe);
}

int32_t TurretRotationController::GetTurretSpeed() 
{ 
    return m_turretSpeedCurrent; 
}

int16_t TurretRotationController::GetTurretAngle()
{
    return m_turretAngleCurrent;
}

void TurretRotationController::SetAutoAimParameters(int32_t p_proportionalConstant, int32_t p_derivativeConstant, int32_t p_steer_max, int32_t p_gyro_gain)
{
    m_pAutoAim->SetParams(p_proportionalConstant, p_derivativeConstant, p_steer_max, p_gyro_gain);  
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
    Telem.SendTurretRotationTelemetry(m_state, m_turretSpeedCurrent);
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
            if (m_turretSpeedCurrent != 0)
            {
                setSpeed(0);
            }
        }
        break;
    }
}

void TurretRotationController::updateAngle()
{
    //  Don't currently have an angle sensor for turret position

    m_turretAngleCurrent = k_invalidTurretAngleRead;
}

void TurretRotationController::setSpeed(int32_t p_speed)
{
    m_turretSpeedCurrent = p_speed;
    m_turretSpeedCurrent = constrain(p_speed, k_minSpeed, k_maxSpeed);

    //  send "@nn!G mm" over software serial. mm is a command 
    //  value, -1000 to 1000. nn is node number in RoboCAN network.
    
    s_TurretRotationMotorSerial.print("@01!G ");
    s_TurretRotationMotorSerial.println(m_turretSpeedCurrent);
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
            m_turretSpeedCurrent = 0;
            m_turretAngleCurrent = k_invalidTurretAngleRead;

            m_pAutoAim = new AutoAim();
            init();
        }
        break;

        case ESafe:
        {
            setSpeed(0);
        }
        break;

        default:
        break;
    }
}

void TurretRotationController::init()
{
    m_pAutoAim->Init();
    initRoboTeq();
}

void TurretRotationController::initRoboTeq()
{
    s_TurretRotationMotorSerial.begin(115200);

    // set serial priority first
    s_TurretRotationMotorSerial.println("@00^CPRI 1 0");
    delay(5);

    // set RC priority second
    s_TurretRotationMotorSerial.println("@00^CPRI 2 1");
    delay(5);

    // turn off command echo
    s_TurretRotationMotorSerial.println("@00^ECHOF 1");
    delay(5);

    // set RS232 watchdog to 100 ms
    s_TurretRotationMotorSerial.println("@00^RWD 100");
}

void TurretRotationController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TurretRotationController::Params));
}