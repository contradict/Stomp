//
//  Turret Rotation Controller
//

#include "Arduino.h"
#include "pins.h"

#include "sbus.h"
#include "telemetryController.h"

#include "turretController.h"
#include "radioController.h"
#include "targetTrackingController.h"
#include "targetAcquisitionController.h"

//  ====================================================================
//
//  File static variables
//
//  ====================================================================

static struct TargetAcquisitionController::Params EEMEM s_savedParams = 
{
    .objectSizeMin = 200,
    .objectSizeMax = 1800,
    .edgeCallThreshold = 60,
    .closestOnly = 1
};

//  ====================================================================
//
//  Public API methods
//
//  ====================================================================

void TargetAcquisitionController::Init()
{
    m_state = EInvalid;
    m_lastUpdateTime = micros();
    setState(EInit);
}

void TargetAcquisitionController::Update()
{
    m_lastUpdateTime = micros();

    //  Normally, we update the state first and then once state settles,
    //  do any other updates that debend on the updated state.  However,
    //  in this case, our state changing is dependant on updateing the
    //  what the best target is, so do that before state stablization.

    updateBestTarget();

    //  Pass update to our owned objects

    //  Update our state

    while(true)
    {
        controllerState prevState = m_state;

        switch (m_state)
        {
            case EInit:
            {
                setState(ENoTargets);
            }
            break;

            case ENoTargets:
            {
                if (m_pBestTarget != NULL)
                {
                    setState(ETargetAcquired);
                }
            }
            break;

            case ETargetAcquired:
            {
                if (m_pBestTarget == NULL)
                {
                    setState(ENoTargets);
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
}

Target* TargetAcquisitionController::GetBestTarget()
{
    return m_pBestTarget;
}

void TargetAcquisitionController::SetParams(int32_t p_objectSizeMin, int32_t p_objectSizeMax, int32_t p_edgeCallThreshold, bool p_closestOnly)
{
    m_params.objectSizeMin = p_objectSizeMin;
    m_params.objectSizeMax = p_objectSizeMax;
    m_params.edgeCallThreshold = p_edgeCallThreshold;
    m_params.closestOnly = p_closestOnly;

    saveParams();
}

void TargetAcquisitionController::RestoreParams()
{
    eeprom_read_block(&m_params, &s_savedParams, sizeof(struct TargetAcquisitionController::Params));
}

void TargetAcquisitionController::SendTelem()
{
    Telem.SendObjectsTelemetry(m_possibleTargetsCount, m_possibleTargets); 
}

void TargetAcquisitionController::SendLeddarTelem()
{
    Telem.SendLeddarTelem(*m_minDetections, m_rawDetectionCount);
}


//  ====================================================================
//
//  Private methods
//
//  ====================================================================

void TargetAcquisitionController::updateBestTarget()
{
    //  We may need to issue another Request for Detections if it has been to loong

    if (m_lastUpdateTime - m_lastRequestDetectionsTime > k_leddarRequestMaxDt)
    {
        m_lastRequestDetectionsTime = m_lastUpdateTime;
        requestDetections();
    }

    //  If not ready to get all the detections forom Leddar, just return
    //  we can pick them up next call

    if (!bufferDetections())
    {
        return;
    }

    m_rawDetectionCount = parseDetections();

    m_lastRequestDetectionsTime = m_lastUpdateTime;
    requestDetections();
    calculateMinimumDetections(m_rawDetectionCount);

    getMinimumDetections(&m_minDetections);

    //  Now that Leddar has given us the detections, segment into m_possibleTargets
    //  then and select the best one and store in m_pBestTarget

    segmentTargets();
    selectTarget();

    //  BB MJS: Debug Print Info for analysis.  Remove
    
    bool validTarget = (m_pBestTarget == NULL);

    String minDetectionsString = "";

    for (uint8_t detectionIndex = 0; detectionIndex < LEDDAR_SEGMENTS; detectionIndex++) 
    {
        minDetectionsString += String((int16_t) (*m_minDetections)[detectionIndex].Segment) + String(", ");
        minDetectionsString += String((int16_t) (*m_minDetections)[detectionIndex].Distance) + String(", ");
        minDetectionsString += String((int16_t) (*m_minDetections)[detectionIndex].Amplitude) + String(", ");
    }

    String possibleTargetsString = "";

    for (uint32_t targetIndex = 0; targetIndex < 8; targetIndex++)
    {
        if (targetIndex < m_possibleTargetsCount)
        {
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetSize()) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetAngle()) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetXCoord()) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetYCoord()) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetRadius()) + String(", ");
        }
        else
        {
            possibleTargetsString += String("-1, -1, -1, -1, -1,");
        }        
    }

    Telem.LogMessage(String("target acquisition: ") + 
        String(m_lastUpdateTime) + String(", ") + 
        String(validTarget) + String(", ") +
        minDetectionsString +
        possibleTargetsString + 
        String(m_possibleTargetsCount));

}

void TargetAcquisitionController::segmentTargets()
{
    // call all objects in frame by detecting edges
    
    int16_t lastSegDistance = (*m_minDetections)[0].Distance;
    int16_t rightEdge = 0;
    int16_t leftEdge = 0;
    uint8_t targetIndex = 0;
    
    // this currently will not call a more distant object obscured by a nearer
    // object, even if both edges of more distant object are visible
    for (uint8_t i = 1; i < LEDDAR_SEGMENTS; i++) 
    {
        int16_t delta = (int16_t) (*m_minDetections)[i].Distance - lastSegDistance;
        
        if (delta < -m_params.edgeCallThreshold) 
        {
            //  transition from further to closer

            leftEdge = i;
            m_possibleTargets[targetIndex].SumDistance = 0;
            m_possibleTargets[targetIndex].SumIntensity = 0;
            m_possibleTargets[targetIndex].SumAngleIntensity = 0;
        } 
        else if (delta > m_params.edgeCallThreshold || i == (LEDDAR_SEGMENTS - 1)) 
        {
            //  transition from closer to further or ran off the right of Leddar FOV
            //  then call it an object if there is an unmatched left edge or the left
            //  edge is 0

            if (leftEdge > rightEdge || leftEdge == 0) 
            {
                rightEdge = i;

                int16_t size = m_possibleTargets[targetIndex].GetSize();
                
                if (size > m_params.objectSizeMin && size < m_params.objectSizeMax) 
                {
                    m_possibleTargets[targetIndex].LeftEdge = leftEdge;
                    m_possibleTargets[targetIndex].RightEdge = rightEdge;
                    m_possibleTargets[targetIndex].Time = m_lastUpdateTime;

                    if (leftEdge == 0 && rightEdge == LEDDAR_SEGMENTS - 1)
                    {   
                        m_possibleTargets[targetIndex].Type = Target::ELeftAndRgihtEdgeOutOfFOV;
                    }
                    if (leftEdge == 0)
                    {   
                        m_possibleTargets[targetIndex].Type = Target::ELeftEdgeOutOfFOV;
                    }
                    else if (rightEdge == LEDDAR_SEGMENTS - 1)
                    {
                        m_possibleTargets[targetIndex].Type = Target::ERightEdgeOutOfFOV;
                    }
                    else
                    {
                        m_possibleTargets[targetIndex].Type = Target::EInFOV;
                    }
                
                    targetIndex++;
                }
            }
        }

        m_possibleTargets[targetIndex].SumDistance += (*m_minDetections)[i].Distance;
        m_possibleTargets[targetIndex].SumIntensity += (*m_minDetections)[i].Amplitude;
        m_possibleTargets[targetIndex].SumAngleIntensity += (int32_t) i * (*m_minDetections)[i].Amplitude;
        lastSegDistance = (*m_minDetections)[i].Distance;
    }

    m_possibleTargetsCount = targetIndex;
}

void TargetAcquisitionController::selectTarget()
{
    bool onlyUseInFOVTargets = false;

    if (m_state != ETargetAcquired)
    {
        //  If we don't currently have a target, then ignore potential targets that 
        //  hang off either the left or right of our FOV.  We only want to go from
        //  ENoTarget -> ETargetAcquired IF we see both edges of the target

        onlyUseInFOVTargets = true;
    }
    else
    {
        //  If we have a current target, then it might now be partially out of FOV and
        //  we want to consider those UNLESS there are any targets that fully in the FOV
        //  in which case just consider EInFOV targets

        for (uint32_t targetIndex = 0; targetIndex < m_possibleTargetsCount; targetIndex++)
        {
            if (m_possibleTargets[targetIndex].Type == Target::EInFOV)
            {
                onlyUseInFOVTargets = true;
                break;
            }
        }
    }
    
    m_pBestTarget = NULL;
    int32_t minTargetDistance = INT32_MAX;

    for (uint32_t targetIndex = 0; targetIndex < m_possibleTargetsCount; targetIndex++)
    {
        if (onlyUseInFOVTargets && m_possibleTargets[targetIndex].Type != Target::EInFOV)
        {
            continue;
        }

        int32_t distance = m_possibleTargets[targetIndex].GetRadius();

        if (distance < minTargetDistance)
        {
            m_pBestTarget = &m_possibleTargets[targetIndex];
            minTargetDistance = distance;
        }
    }
}

/*
void TargetAcquisitionController::selectTarget()
{
    m_pBestTarget = NULL;
    int32_t minTargetDistanceSq = INT32_MAX;

    if (TargetTracking.IsTrackingValidTarget()) 
    {
        for (uint32_t targetIndex = 0; targetIndex < m_possibleTargetsCount; targetIndex++) 
        {
            int32_t distanceSq;
            distanceSq = TargetTracking.GetDistanceSqToTarget(m_possibleTargets[targetIndex]);

            if (distanceSq < minTargetDistanceSq) 
            {
                m_pBestTarget = &m_possibleTargets[targetIndex];
                minTargetDistanceSq = distanceSq;
            }
        }
    } 
    else 
    {
        selectClosestTarget();
    }
}

void TargetAcquisitionController::selectClosestTarget()
{
    m_pBestTarget = NULL;
    int32_t minTargetDistanceSq = INT32_MAX;

    for (uint32_t targetIndex = 0; targetIndex < m_possibleTargetsCount; targetIndex++)
    {
        int32_t distanceSq;
        distanceSq = m_possibleTargets[targetIndex].GetRadius();
        distanceSq *= distanceSq;

        if (distanceSq < minTargetDistanceSq)
        {
            m_pBestTarget = &m_possibleTargets[targetIndex];
            minTargetDistanceSq = distanceSq;
        }
    }
}
*/

void TargetAcquisitionController::setState(controllerState p_state)
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

void TargetAcquisitionController::init()
{
    m_pBestTarget = NULL;
    
    m_lastRequestDetectionsTime = 0;
    m_rawDetectionCount = 0;
    m_possibleTargetsCount = 0;
}

void TargetAcquisitionController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TargetAcquisitionController::Params));
}