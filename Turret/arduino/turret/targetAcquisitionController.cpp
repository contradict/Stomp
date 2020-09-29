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
    Telem.SendObjectsTelemetry(m_possibleTargetsCount, m_bestTargetIndex, m_possibleTargets); 
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
        Telem.LogMessage("TImeout Retry - Request Detections: " + String(m_lastUpdateTime));

        requestDetections();
    }

    //  If not ready to get all the detections forom Leddar, just return
    //  we can pick them up next call

    Telem.LogMessage("Buffer Detections: " + String(m_lastUpdateTime));
    if (!bufferDetections())
    {
        return;
    }

    m_rawDetectionCount = parseDetections();

    getMinimumDetections(&m_minDetections);

    //  Now that Leddar has given us the detections, segment into m_possibleTargets
    //  then and select the best one and store in m_pBestTarget

    segmentTargets();
    selectTarget();

    //  BB MJS: Debug Print Info for analysis.  Remove
    
    bool validTarget = (m_pBestTarget != NULL);

    String minDetectionsString = "";

    for (uint8_t detectionIndex = 0; detectionIndex < LEDDAR_SEGMENTS; detectionIndex++) 
    {
        minDetectionsString += String((int16_t) (*m_minDetections)[detectionIndex].Distance) + String(", ");
        minDetectionsString += String((int16_t) (*m_minDetections)[detectionIndex].Amplitude) + String(", ");
    }

    String possibleTargetsString = "";

    for (uint32_t targetIndex = 0; targetIndex < k_maxPossibleTargets; targetIndex++)
    {
        if (targetIndex < m_possibleTargetsCount)
        {
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetSize()) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].GetDistance() / 10) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].LeftEdge) + String(", ");
            possibleTargetsString += String((int16_t) m_possibleTargets[targetIndex].RightEdge) + String(", ");
        }
        else
        {
            possibleTargetsString += String("-1, -1, -1, -1, -1,");
        }        
    }

    Telem.LogMessage(String("target acquisition, ") + 
        String(m_lastUpdateTime) + String(", ") + 
        String(validTarget) + String(", ") +
        String(m_possibleTargetsCount) + String(", ") +
        minDetectionsString +
        possibleTargetsString);

    //  Issue the next request for detections
    
    m_lastRequestDetectionsTime = m_lastUpdateTime;
    Telem.LogMessage("Request Detections: " + String(m_lastUpdateTime));
    requestDetections();
}

void TargetAcquisitionController::resetTargets()
{    
    for (uint8_t targetIndex = 0; targetIndex < k_maxPossibleTargets; targetIndex++) 
    {
        m_possibleTargets[targetIndex].Reset();
    }
}

//
//  SegmentTargets
//
//  Go through all 16 lidar detections, looking for transformations from far to near
//  or near to far.
//      
//  * Any far to near (or first segment) triggers a start segment
//  * The frist transition from near to far (or last segment) triggers an end segment
//
//  Call it a valid target IF we had both a start and end segment && target is not TOO big
//
//  This currently will not find a more distant target obscured by a nearer
//  target, even if both edges of more distant target are visible
//

void TargetAcquisitionController::segmentTargets()
{
    resetTargets();

    //  Always start target 0 off to the left of FOV

    uint8_t targetIndex = 0;
    m_possibleTargets[targetIndex].StartSegment(0, &(*m_minDetections)[0]);

    //  Go through the reset of the leddar segments

    for (uint8_t segmentIndex = 1; segmentIndex < LEDDAR_SEGMENTS; segmentIndex++) 
    {
        int16_t delta = (*m_minDetections)[segmentIndex].Distance - (*m_minDetections)[segmentIndex - 1].Distance;
        
        if (delta < -m_params.edgeCallThreshold) 
        {
            //  Transion from far to near, start a possible valid target, even if we already 
            //  started a target previously 

            m_possibleTargets[targetIndex].StartSegment(segmentIndex, &(*m_minDetections)[segmentIndex]);
        } 
        else if (delta > m_params.edgeCallThreshold || segmentIndex == (LEDDAR_SEGMENTS - 1)) 
        {
            int16_t rightEdge = segmentIndex;

            if (delta <= m_params.edgeCallThreshold)
            {
                //  If we are accepting this and a target, because it extends
                //  out of FOV, then the right edge of the target is 16 (not 15)
                //  so the Target can correctly count how many segments it covers.
            
                rightEdge = LEDDAR_SEGMENTS;
            }

            //  Transition from near to far or ran off the right of Leddar FOV.
            //  Call it an object if target is still under construction and not too big
            
            int16_t size = m_possibleTargets[targetIndex].GetSize();

            if (m_possibleTargets[targetIndex].Type == Target::EBeingConstructed && 
                size > m_params.objectSizeMin && size < m_params.objectSizeMax)
            {
                m_possibleTargets[targetIndex].EndSegment(rightEdge, &(*m_minDetections)[segmentIndex], m_lastUpdateTime);
                targetIndex++;
            }
        }
        else
        {
            m_possibleTargets[targetIndex].AddSegment(segmentIndex, &(*m_minDetections)[segmentIndex]);
        }
    }

    m_possibleTargetsCount = targetIndex;
}

void TargetAcquisitionController::selectTarget()
{
    //  First determaine if we will consider targets that are partially 
    //  out of our FOV.

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
    
    //  Now, get the best target

    m_pBestTarget = NULL;
    m_bestTargetIndex = -1;

    int32_t minTargetDistance = INT32_MAX;

    for (uint8_t targetIndex = 0; targetIndex < m_possibleTargetsCount; targetIndex++)
    {
        if (onlyUseInFOVTargets && m_possibleTargets[targetIndex].Type != Target::EInFOV)
        {
            continue;
        }

        int32_t distance = m_possibleTargets[targetIndex].GetDistance();

        if (distance < minTargetDistance)
        {
            m_bestTargetIndex = targetIndex;
            m_pBestTarget = &m_possibleTargets[m_bestTargetIndex];
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
        distanceSq = m_possibleTargets[targetIndex].GetDistance();
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
            m_lastRequestDetectionsTime = m_lastUpdateTime;
            Telem.LogMessage("Request Detections: " + String(m_lastUpdateTime));
            requestDetections();
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

    m_bestTargetIndex = -1;
}

void TargetAcquisitionController::saveParams() 
{
    eeprom_write_block(&m_params, &s_savedParams, sizeof(struct TargetAcquisitionController::Params));
}