//
//  IMU Controller
//

#include "Arduino.h"
#include "pins.h"

#include "I2C.h"
#include "MPU6050.h"
#include "telemetryController.h"

#include "fixedpoint.h"
#include "turretController.h"
#include "radioController.h"
#include "imuController.h"

//  ====================================================================
//
//  File constants 
//
//  ====================================================================

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static MPU6050 s_IMUSerial;

static struct IMUController::Params EEMEM s_savedParams = 
{
    .dlpfMode=MPU6050_DLPF_BW_20,
    .imuPeriod=100000,
    .stationaryThreshold=200,
    .uprightCross = 410,
    .minValidCross = 820,
    .maxValidCross = 2458,
    .maxTotalNorm = 3584,
    .xThreshold = 1229,
    .zThreshold = 2028
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

void IMUController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void IMUController::Update()
{
    m_lastUpdateTime = micros();

    //  Pass update to our owned objects

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
                    setState(EUnknown);
                }
            }
            break;

            case EUnknown:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (doesMathSayUpright())
                {
                    setState(EUpright);
                } 
                else if (doesMathSayNotUpright())
                {
                    setState(ENotUpright);
                }
            }
            break;

            case EUpright:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (m_queryIMUFailed)
                {
                    setState(EUnknown);
                }
                else if (doesMathSayNotUpright())
                {
                    setState(ENotUpright);
                }
                else if (!doesMathSayUpright())
                {
                    setState(EUnknown);
                } 
            }
            break;

            case ENotUpright:
            {
                if (!Radio.IsNominal())
                {
                    setState(ESafe);
                }
                else if (m_queryIMUFailed)
                {
                    setState(EUnknown);
                }
                else if (doesMathSayUpright())
                {
                    setState(EUpright);
                }
                else if (!doesMathSayNotUpright())
                {
                    setState(EUnknown);
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

    queryIMU();
}

int16_t IMUController::GetOmegaZ() 
{
    if(m_queryIMUFailed) 
    {
        return 0;
    }

    return m_angularRate[2];
}

bool IMUController::IsUpright()
{
    return m_state = EUpright;
}

void IMUController::SetParams(int8_t p_dlpf, int32_t p_imuPeriod, int32_t p_stationaryThreshold,
    int16_t p_uprightCross, int16_t p_minValidCross, int16_t p_maxValidCross,
    int16_t p_maxTotalNorm, int16_t p_xThreshold, int16_t p_zThreshold)
{
    m_params.dlpfMode = p_dlpf;
    m_params.imuPeriod = p_imuPeriod;
    m_params.stationaryThreshold = p_stationaryThreshold;
    m_params.uprightCross = p_uprightCross;
    m_params.minValidCross = p_minValidCross;
    m_params.maxValidCross = p_maxValidCross;
    m_params.maxTotalNorm = p_maxTotalNorm;
    m_params.xThreshold = p_xThreshold;
    m_params.zThreshold = p_zThreshold;

    saveParams();    
}

void IMUController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct IMUController::Params));
}

void IMUController::SendTelem()
{
    Telem.SendIMUTelem(m_acceleration, m_angularRate, m_temperature);
    Telem.SendORNTelem(m_state == EUpright || m_state == ENotUpright, m_state, m_sumAngularRate, m_totalNorm, m_crossNorm);
}

//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void IMUController::queryIMU()
{
    //  Only query IMU at the requested period

    if (m_lastUpdateTime - m_lastIMUQueryTime < m_params.imuPeriod)
    {
        return;
    }

    m_lastIMUQueryTime= micros();

    if (s_IMUSerial.getMotion6(&m_acceleration[0], &m_acceleration[1], &m_acceleration[2],
                               &m_angularRate[0], &m_angularRate[1], &m_angularRate[2]) != 0)
    {
        m_queryIMUFailed = true;
        return;
    }
        
    m_queryIMUFailed = false;

    m_temperature = s_IMUSerial.getTemperature();

    m_sumAngularRate = (abs(m_angularRate[0]) + abs(m_angularRate[1]) + abs(m_angularRate[2]));

    m_totalNorm = (int32_t)m_acceleration[0] * m_acceleration[0] / 2048 +
        (int32_t)m_acceleration[1] * m_acceleration[1] / 2048 +
        (int32_t)m_acceleration[2] * m_acceleration[2] / 2048;

    if (m_sumAngularRate < m_params.stationaryThreshold && m_totalNorm < m_params.maxTotalNorm)
    {
        // Compute cross product with Zhat
        // a = measured
        // b = [0, 0, 1]
        // cx = ay*bz - az*by
        // cy = az*bx - ax*bz
        // cz = ax*by - ay*bx
    
        int32_t crossX =  m_acceleration[1]; // * 1.0 in z
        int32_t crossY = -m_acceleration[0];
 
        m_crossNorm = crossX * crossX / 2048 + crossY * crossY / 2048;
    }
}

bool IMUController::isPossiblyStationary()
{
    return !m_queryIMUFailed && m_sumAngularRate < m_params.stationaryThreshold && m_totalNorm < m_params.maxTotalNorm;
}

bool IMUController::doesMathSayUpright()
{
    if (isPossiblyStationary())
    {
        if (m_crossNorm < m_params.uprightCross && m_acceleration[2] > m_params.zThreshold)
        {
            return true;
        }
    }

    return false;
}

bool IMUController::doesMathSayNotUpright()
{
    if (doesMathSayUpright())
    {
        return false;
    }

    if (isPossiblyStationary())
    {
        if (m_crossNorm > m_params.minValidCross && m_crossNorm < m_params.maxValidCross) 
        {
            if (abs(m_acceleration[0]) > m_params.xThreshold)
            {
                return true;
            }
        } 
    }

    return false;
}

void IMUController::setState(controllerState p_state)
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
            init();
        }
        break;

        default:
        break;
    }
}

void IMUController::init()
{
    I2c.begin();
    I2c.setSpeed(false);
    I2c.timeOut(2);

    s_IMUSerial.initialize();

    Telem.LogMessage(String("IMU.getDeviceID() = ") + s_IMUSerial.getDeviceID());

    s_IMUSerial.setFullScaleGyroRange(MPU6050_GYRO_FS_2000);
    s_IMUSerial.setFullScaleAccelRange(MPU6050_ACCEL_FS_16);
    s_IMUSerial.setDLPFMode(MPU6050_DLPF_BW_20);

    m_lastIMUQueryTime = 0;
}

void IMUController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct IMUController::Params));
}