#include "Arduino.h"
#include "telemetryController.h"

#include "target.h"

void Target::Reset()
{
    Type = EInvalid;    
    LeftEdge = 0;
    RightEdge = 0;

    SumDistance = 0;
    SumIntensity = 0;
    SumAngleIntensity = 0;
}

void Target::StartSegment(int32_t p_startSegment, Detection* p_detection)
{
    Type = EBeingConstructed;
    LeftEdge = p_startSegment;

    //  Convert distances from cm to mm
    SumDistance = p_detection->Distance * 10;

    SumIntensity = p_detection->Amplitude;
    SumAngleIntensity = p_startSegment * p_detection->Amplitude;
}

void Target::AddSegment(int32_t p_segment, Detection* p_detection)
{
    //  Convert distances from cm to mm
    SumDistance += p_detection->Distance * 10;

    SumIntensity += p_detection->Amplitude;
    SumAngleIntensity += p_segment * p_detection->Amplitude;
}

void Target::EndSegment(int32_t p_endSegment, Detection* p_detection, uint32_t p_time)
{
    RightEdge = p_endSegment;
    Time = p_time;

    if (LeftEdge == 0 && RightEdge == LEDDAR_SEGMENTS - 1)
    {   
        Type = ELeftAndRgihtEdgeOutOfFOV;
    }
    else if (LeftEdge == 0)
    {   
        Type = ELeftEdgeOutOfFOV;
    }
    else if (RightEdge == LEDDAR_SEGMENTS - 1)
    {
        Type = ERightEdgeOutOfFOV;
    }
    else
    {
        Type = EInFOV;
    }
}

//  Estimate the size of a target as the sum of the arc lengh of each segment
//  Arc length of circle given theta in radians is simpley s = theta * radius
//
//  * Take the average distance to the target and use as radius
//  * The degrees each segment of the leddar covers is 99 deg FOV / 16 segments => 6.1875 deg or 0.108 rad
//  * Theta therefore is fixed for each segment at 0.108 rad
//
//  Return size in mm
int16_t Target::GetSize() 
{
    //  BB MJS: convert this code to use fixed point
    float size = GetDistance() * ((RightEdge - LeftEdge) * 0.108f);
    return (int16_t) size;
}

// return average distance to target in mm
int16_t Target::GetDistance() 
{
    return SumDistance / (RightEdge - LeftEdge);
}

// angle in radians scaled by 2048
int16_t Target::GetAngle() 
{
    // 2048*.108 = 221.2
    // 7.5*221.2 = 1659
    return - SumAngleIntensity * (int32_t)221 / SumIntensity + (int32_t)1659;
}

// x coordinate in mm
int16_t Target::GetXCoord() 
{
    int32_t angle = GetAngle();
    int32_t distance = GetDistance();

    int16_t xCoord = (int16_t)((distance * (2048L - ((angle * angle) / 4096L))) / 2048L);

    /*
    float degrees = ((float)angle / 2048.0f) * (180.0f/PI);
    Telem.LogMessage(String("xCoord = ") + String(xCoord) + 
        String(" angle = ") + String(degrees) + 
        String(" distance = ") + String(distance));
    */
       
    return xCoord;
}

// y coordinate in mm
int16_t Target::GetYCoord() 
{
    return ((int32_t)GetDistance()*GetAngle())/2048L;
}

// distance squared in mm
inline int32_t Target::DistanceSq(Target &p_otherTarget) 
{
    int32_t r = GetDistance();
    int32_t a = GetAngle();

    int32_t ro=p_otherTarget.GetDistance();
    int32_t ao=p_otherTarget.GetAngle();

    // x = radius()*cos(angle());
    // y = radius()*sin(angle());
    // xo = p_otherTarget.radius()*cos(p_otherTarget.angle());
    // yo = p_otherTarget.radius()*sin(p_otherTarget.angle());
    // d = (x-xo)**2 + (y-yo)**2;
    // d = r*r*ca*ca - 2*r*ca*ro*cao + ro*ro*cao*cao + r*r*sa*sa - 2*r*sa*ro*sao + ro*ro*sao*sao;
    // d = r*r + ro*ro - 2*r*ro*(ca*cao + sa*sao);
    // angle in 1/2 miliradians, cos(angle) = (2048-angle*angle/2048/2)/2048
    //                           sin(angle) = angle/2048
    // d = r*r + ro*ro - 2*r*ro*((2048-a*a/2/2048)*(2048-ao*ao/2/2048)/2048/2048 + a*ao/2048/2048);
    // d = r*r + ro*ro - 2*r*ro*((2048-a*a/2/2048)*(2048-ao*ao/2/2048) + a*ao)/2048/2048;
    // d = r*r + ro*ro - 2*r*ro*(2048*2048 - a*a/2 - ao*ao/2 + a*a*ao*a0/2/2/2048/2048)/2048/2048;
    // d = r*r + ro*ro - r*ro*(4096*2048 - a*a - ao*ao + a*a*ao*a0/2/2048/2048)/2048/2048;

    return r*r + ro*ro - r*ro*(8388608L - a*a - ao*ao + a*a*ao*ao/8388608L)/4194304L;
}

