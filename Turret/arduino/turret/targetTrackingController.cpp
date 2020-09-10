//
//  Turret Tracking Controller
//

#include "Arduino.h"
#include "pins.h"

#include "telemetryController.h"

#include "turretController.h"
#include "radioController.h"
#include "targetAcquisitionController.h"

#include "targetTrackingController.h"

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static struct TargetTrackingController::Params EEMEM s_savedParams = 
{
    .alpha = 9000,
    .beta = 8192,
    .trackLostDt = 250000,
    .minNumUpdates = 3,
    .maxOffTrackDistanceSq = 1000L * 1000L,
    .maxStartDistanceSq = 6000L * 6000L
};

//  ====================================================================
//
//  Public API methods
//
//  ====================================================================

void TargetTrackingController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void TargetTrackingController::Update()
{
    uint32_t previousUpdateTime = m_lastUpdateTime;

    m_lastUpdateTime = micros();
    m_latestDt = m_lastUpdateTime - previousUpdateTime;

    //  Need to "predict" before passing along the Update to Acquisition Controller

    predictedTrackedTargetLocation();

    //  Pass update to our owned objects

    m_pTargetAcquisitionController->Update();

    //  Grab the target that the Target Acquisition Controller believes is the best
    //  this maybe NULL if there are not targets

    Target *pBestTarget = m_pTargetAcquisitionController->GetBestTarget();

    //  Update our state

    while(true)
    {
        controllerState prevState = m_state;

        switch (m_state)
        {
            case EInit:
            {
                setState(ENoTarget);
            }
            break;

            case ENoTarget:
            {
                if (pBestTarget != NULL)
                {
                    setState(ETargetAcquired);
                }
            }
            break;

            case ETargetAcquired:
            {
                if (pBestTarget == NULL || 
                    pBestTarget != m_pTrackedTarget) /* BB MJS: ||
                    m_latestDt > m_params.trackLostDt || 
                    GetDistanceSqToTarget(*m_pTrackedTarget) > m_params.maxOffTrackDistanceSq)*/
                {
                    setState(ENoTarget);
                }
                else if (m_numUpdates >= m_params.minNumUpdates)
                {
                    setState(ETargetTracked);
                }
            }
            break;
            
            case ETargetTracked:
            {
                if (pBestTarget == NULL || 
                    pBestTarget != m_pTrackedTarget) /* BB MJS: ||
                    m_latestDt > m_params.trackLostDt || 
                    GetDistanceSqToTarget(*m_pTrackedTarget) > m_params.maxOffTrackDistanceSq)*/
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

    //  Now that the state is stable, take action based on stable state

    updateTracking();
}

Target* TargetTrackingController::GetCurrentTarget()
{
    if (!IsTrackingValidTarget())
    {
        return NULL;
    }

    return m_pTrackedTarget;
}

bool TargetTrackingController::IsTrackingValidTarget()
{
    return m_pTrackedTarget != NULL && m_state == ETargetTracked;
}

bool TargetTrackingController::WillHitTrackedTarget(int32_t p_xTolerance, int32_t p_yTolerance)
{
    if (!IsTrackingValidTarget())
    {
        Telem.LogMessage(String("will hit: ") + 
        String(m_lastUpdateTime) + String(", ") + 
        String(false) + String(", ") + 
        String("-1, ") +
        String("-1, ") +
        String("-1, ") +
        String("-1, ") +
        String("-1, ") +
        String("-1"));

        return false;
    }

    int32_t projectedX = m_x;
    int32_t projectedY = m_y;
    int32_t omegaZ = Turret.GetTurretRotationSpeed();

    int32_t swingDt = Turret.GetEstimatedSwingDuration();
    int32_t projectDt = swingDt / k_willHitProjectSteps;

    for (int step = 0; step < k_willHitProjectSteps; step++) 
    {
        int32_t dTheta = (projectDt * omegaZ) / 1000000;
        project(projectDt, dTheta, &projectedX, &projectedY);
    }

    bool willHit = projectedX > 0 && (projectedX / 16) < p_xTolerance && abs(projectedY / 16) < p_yTolerance;

    Telem.LogMessage(String("will hit, ") + 
        String(m_lastUpdateTime) + String(", ") + 
        String(willHit) + String(", ") + 
        String(swingDt) + String(", ") +
        String(omegaZ) + String(", ") +
        String(projectedX) + String(", ") +
        String(projectedY) + String(", ") +
        String(p_xTolerance) + String(", ") +
        String(p_yTolerance));

    return willHit;
}

int32_t TargetTrackingController::GetTargetErrorAngle(void)
{
    //  Return the angle between where the turret is pointing and where we 
    //  think the target is

    if (IsTrackingValidTarget())
    {
        return getAngle();
    }

    return 0;
}

int32_t TargetTrackingController::GetDistanceSqToTarget(const Target &p_target) 
{
    // distance squared to target in mm

    int32_t dx = (m_x / 16) - p_target.GetXCoord();
    int32_t dy = (m_y / 16) - p_target.GetYCoord();

    return dx*dx + dy*dy;
}

void TargetTrackingController::SetObjectSegmentationParams(int32_t p_objectSizeMin, int32_t p_objectSizeMax, int32_t p_edgeCallThreshold, bool p_closestOnly)
{
    m_pTargetAcquisitionController->SetParams(p_objectSizeMin, p_objectSizeMax, p_edgeCallThreshold, p_closestOnly);
}

void TargetTrackingController::SetParams(int16_t p_alpha, 
    int16_t p_beta,
    int8_t p_minNumUpdates,
    uint32_t p_trackLostDt,
    int16_t p_maxOffTrackDistance,
    int16_t p_maxStartDistance)
{
    m_params.alpha = p_alpha;
    m_params.beta = p_beta;
    m_params.minNumUpdates = p_minNumUpdates;
    m_params.trackLostDt = p_trackLostDt;
    m_params.maxOffTrackDistanceSq = p_maxOffTrackDistance * p_maxOffTrackDistance;
    m_params.maxStartDistanceSq = p_maxStartDistance * p_maxStartDistance;

    saveParams();
}

void TargetTrackingController::RestoreParams()
{
    m_pTargetAcquisitionController->RestoreParams();

    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TargetTrackingController::Params));
}

void TargetTrackingController::SendTelem()
{
    m_pTargetAcquisitionController->SendTelem();

    if (IsTrackingValidTarget())
    {
        Telem.SendTrackingTelemetry(
                m_state,
                m_pTrackedTarget->GetXCoord(), m_pTrackedTarget->GetYCoord(),
                m_pTrackedTarget->GetAngle(), m_pTrackedTarget->GetDistance(),
                m_x / 16, m_vx / 16,
                m_y / 16, m_vy / 16);
    }
    else
    {
        Telem.SendTrackingTelemetry(
                m_state,
                0, 0, 0, 0,
                m_x / 16, m_vx / 16,
                m_y / 16, m_vy / 16);
    }
}

void TargetTrackingController::SendLeddarTelem()
{
    m_pTargetAcquisitionController->SendLeddarTelem();
}


//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TargetTrackingController::predictedTrackedTargetLocation() 
{
    if (m_pTrackedTarget == NULL)
    {
        return;
    }

    // return change in body angle as measured by gyro
    // radians scaled by 2048
    // work with radians/sec scaled by 32768
    // (2000deg/sec)/(32768 full scale)*pi/180*32768 = 34.9

    int32_t converted = Turret.GetTurretRotationSpeed() * 35;
    int32_t average_omegaZ = (m_lastOmgaZ + converted) / 2;

    m_lastOmgaZ = converted;

    int32_t dtheta = ((average_omegaZ / 16) * (((int32_t) m_latestDt) / 1000)) / 1000;

    project(m_latestDt, dtheta, &m_x, &m_y);

    /*
    Telem.LogMessage(String("dt: ") + String(m_latestDt) + String(" ") +
                String("average_omegaZ: ") + String(average_omegaZ) + String(" ") +
                String("dtheta: ") + String(dtheta) + String(" ") +
                String("m_x: ") + String(m_x) + String(" ") +
                String("m_y: ") + String(m_y) + String(" ") +
                String("error: ") + String(GetTargetErrorAngle()));
    */
}

void TargetTrackingController::updateTracking()
{
    if (m_pTrackedTarget != NULL)
    {
        m_numUpdates++;
    }

    if (!IsTrackingValidTarget())
    {
        m_residualX = 0;
        m_residualY = 0;
        m_x = 0;
        m_y = 0;
        m_vx = 0;
        m_vy = 0;

        Telem.LogMessage(String("tracking, ") + 
            String(m_lastUpdateTime) + String(", ") + 
            String("-1, ") +
            String("-1, ") +
            String("0, ") +
            String("0, ") +
            String("0, ") +
            String("0, ") +
            String("0, ") +
            String("0, ") +
            String(m_numUpdates) + String(", ") +
            String(m_params.alpha) + String(", ") +
            String(m_params.beta));

        return;
    }

    //  Alpha Beta Filter https://en.wikipedia.org/wiki/Alpha_beta_filter
    //  to get new position and velocity

    int32_t measuredX = (int32_t)(m_pTrackedTarget->GetXCoord()) * 16;
    int32_t measuredY = (int32_t)(m_pTrackedTarget->GetYCoord()) * 16;
 
    // residual:
    // rx = mr*cos(ma) - x
    // rx = mr*(2048 - ma*ma/2048/2)/2048 - x
    // ry = mr*sin(ma) - y

    m_residualX = measuredX - m_x;
    m_residualX = constrain(m_residualX, -65535L, 65535L);

    m_residualY = measuredY - m_y;
    m_residualY = constrain(m_residualY, -65535L, 65535L);

    // correct:

    m_x += m_params.alpha * m_residualX / 32767;
    m_y += m_params.alpha * m_residualY / 32767;

    m_vx += m_params.beta * m_residualX / 4096;
    m_vx = constrain(m_vx, -10000L * 16, 10000L * 16);

    m_vy += m_params.beta * m_residualY / 4096;
    m_vy = constrain(m_vy, -10000L * 16, 10000L * 16);

    Telem.LogMessage(String("tracking, ") + 
        String(m_lastUpdateTime) + String(", ") + 
        String(m_numUpdates) + String(", ") +
        String(measuredX) + String(", ") +
        String(measuredY) + String(", ") +
        String(m_residualX) + String(", ") +
        String(m_residualY) + String(", ") +
        String(m_x) + String(", ") +
        String(m_y) + String(", ") +
        String(m_vx) + String(", ") + 
        String(m_vy) + String(", "));
    /*
    Telem.LogMessage(String("measuredX: ") + String(measuredX) + String(" ") +
                String("measuredY: ") + String(measuredY) + String(" ") +
                String("m_residualX: ") + String(m_residualX) + String(" ") +
                String("m_residualY: ") + String(m_residualY) + String(" ") +
                String("m_x: ") + String(m_x) + String(" ") +
                String("m_y: ") + String(m_y) + String(" ") +
                String("m_vx: ") + String(m_vx) + String(" ") +
                String("m_vy: ") + String(m_vy) + String(" ") +
                String("alpha: ") + String(m_params.alpha) + String(" ") +
                String("beta: ") + String(m_params.beta) + String(" "));
    */
}


void TargetTrackingController::project(int32_t dt, int32_t dtheta, int32_t *px, int32_t *py) 
{
    //  BB MJS: Need to figure this out and document
    //  Also is DT in millisecond or micro seconds?

    // predict:
    // r = sqrt(x**2+y**2)
    // theta = atan2(y, x)
    // x = x + dt*vx/1e6 + r*(cos(theta-dtheta) - cos(theta))
    // x = x + dt*vx/1e6 + r*(cos(theta)*cos(dtheta) + sin(theta)*sin(dtheta) - x/r)
    // x = x + dt*vx/1e6 + r*((x/r)*cos(dtheta) + (y/r)*sin(dtheta) - x/r)
    // x = x + dt*vx/1e6 + (x*cos(dtheta) + y*sin(dtheta) - x)
    // x = x + dt*vx/1e6 + (x*(cos(dtheta)-1) + y*sin(dtheta))
    // x = x + dt*vx/1e6 + (x*(-dtheta*dtheta/2048/2/2048) + y*dtheta/2048)
    // x = x + dt*vx/1e6 - (x*dtheta*dtheta/2048/2 - y*dtheta)/2048
    int32_t lx=*px;
    int32_t ly=*py;
    *px = lx + ((dt / 1000) * m_vx) / 1000 - ((lx*dtheta/1024L)*dtheta/4L - ly*dtheta)/2048L;
    // y = y + dt*vy/1e6 + r*(sin(theta-dtheta) - sin(theta));
    // y = y + dt*vy/1e6 + r*(sin(theta)*cos(dtheta) - cos(theta)*sin(dtheta) - sin(theta));
    // y = y + dt*vy/1e6 + r*((y/r)*cos(dtheta) - (x/r)*sin(dtheta) - (y/r));
    // y = y + dt*vy/1e6 + (y*cos(dtheta) - x*sin(dtheta) - y);
    // y = y + dt*vy/1e6 + (y*(cos(dtheta)-1) - x*sin(dtheta));
    // y = y + dt*vy/1e6 - (y*(dtheta*dtheta/2048/2) + x*dtheta)/2048;
    *py = ly + (( dt / 1000) * m_vy) / 1000 - ((ly*dtheta/1024L)*dtheta/4L + lx*dtheta)/2048L;
}

int32_t TargetTrackingController::getAngle(void)
{
    //  Returns the angle between forward (the axis of the hammer) and the target
    //  Angle is in fixed point (1:20:11) raidans (float scaled by 2048)

    if (IsTrackingValidTarget()) 
    {
        // arctan(y/x)
        return ((m_y*2048L)/ m_x - ((((((m_y*(m_y/16L))/m_x)*m_y)/m_x)*2048L)/m_x)*16L/3L);
    } 
    else 
    {
        return 0;
    }
}

int32_t TargetTrackingController::getVTheta(void) 
{
    if (IsTrackingValidTarget()) 
    {
        // d/dt arctan(y/x) = (1/(1+(y/x)**2))*(vy/x - y*vx/x**2)
        // (vy*x-vx*y)/(x**2+y**2)
        return m_vy * 2048 / m_x; // (vy*x - vx*y)*128L/(x*x/16L + y*y/16L);
    } 
    else 
    {
        return 0;
    }
}

void TargetTrackingController::setState(controllerState p_state)
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
            initAllControllers();
        }
        break;

        case ENoTarget:
        {
            m_pTrackedTarget = NULL;
        }
        break;
        
        case ETargetAcquired:
        {
            m_pTrackedTarget = m_pTargetAcquisitionController->GetBestTarget();

            m_x = m_pTrackedTarget->GetXCoord() * 16;
            m_y = m_pTrackedTarget->GetYCoord() * 16;
            m_vx = 0;
            m_vy = 0;
            m_numUpdates = 0;
        }
        break;

        default:
        break;
    }
}

void TargetTrackingController::init()
{
    m_pTargetAcquisitionController = new TargetAcquisitionController();

    m_x = 0;
    m_vx = 0;
    m_y = 0;
    m_vy = 0;
    m_residualX = 0;
    m_residualY = 0;
    m_numUpdates = 0;
    m_lastOmgaZ = 0;
}

void TargetTrackingController::initAllControllers()
{
    m_pTargetAcquisitionController->Init();
}

void TargetTrackingController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TargetTrackingController::Params));
}