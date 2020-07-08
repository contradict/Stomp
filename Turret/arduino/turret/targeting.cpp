#include "Arduino.h"
#include "leddar_io.h"
#include "targeting.h"
#include "pins.h"
#include <stdlib.h>
#include <math.h>
#include "imu.h"
#include "telemetryController.h"
#include "utils.h"

static void saveObjectSegmentationParameters();

static int8_t selectObject(const Object (&objects)[8], uint8_t num_objects,
                           const struct Track &tracked_object,
                           int32_t *selected_distance);

static int8_t selectClosestObject(const Object (&objects)[8], uint8_t num_objects,
                           const struct Track &tracked_object,
                           int32_t *selected_distance);

struct ObjectSegmentationParameters {
    int32_t min_object_size;     // object sizes are in mm
    int32_t max_object_size;     // mm of circumferential size
    int32_t edge_call_threshold; // cm for edge in leddar returns
    bool closest_only;           // Only inspect closest object
} __attribute__((packed));

static struct ObjectSegmentationParameters object_params;

struct ObjectSegmentationParameters EEMEM saved_object_params = {
    .min_object_size = 200,    // object sizes are in mm
    .max_object_size = 1800,   // mm of circumferential size
    .edge_call_threshold = 60, // cm for edge in leddar returns
    .closest_only = 1          // Only inspect closest object
};

int8_t trackObject(uint32_t now, struct Object (&objects)[8], uint8_t num_objects,
                    struct Track& tracked_object) {

    int16_t omegaZ = 0;
    getOmegaZ(&omegaZ);


    int8_t best_object = -1;
    if(num_objects>0) {
        tracked_object.predict(now, omegaZ);
        int32_t best_distance;
        if(object_params.closest_only)
        {
            best_object = selectClosestObject(objects, num_objects, tracked_object, &best_distance);
        }
        else
        {
            best_object = selectObject(objects, num_objects, tracked_object, &best_distance);
        }
        uint32_t now = objects[best_object].Time;

        if(tracked_object.wants_update(now, best_distance)) {
           tracked_object.update(objects[best_object]);
        }
    // below is called if no objects called in current Leddar return
    } else {
        tracked_object.updateNoObs(micros(), omegaZ);
    }
    return best_object;
}


uint8_t segmentObjects(const Detection (&min_detections)[LEDDAR_SEGMENTS],
                              uint32_t now,
                              Object (&objects)[8]) {
    // call all objects in frame by detecting edges
    int16_t last_seg_distance = min_detections[0].Distance;
    int16_t right_edge = 0;
    int16_t left_edge = 0;
    uint8_t num_objects = 0;
    // this currently will not call a more distant object obscured by a nearer
    // object, even if both edges of more distant object are visible
    for (uint8_t i = 1; i < LEDDAR_SEGMENTS; i++) {
        int16_t delta = (int16_t) min_detections[i].Distance - last_seg_distance;
        if (delta < -object_params.edge_call_threshold) {
            left_edge = i;
            objects[num_objects].SumDistance = 0;
            objects[num_objects].SumIntensity = 0;
            objects[num_objects].SumAngleIntensity = 0;
        } else if (delta > object_params.edge_call_threshold) {
            // call object if there is an unmatched left edge
            if (left_edge > right_edge) {
                right_edge = i;
                objects[num_objects].LeftEdge = left_edge;
                objects[num_objects].RightEdge = right_edge;
                objects[num_objects].Time = now;
                int16_t size = objects[num_objects].size();
                if(size>object_params.min_object_size &&
                   size<object_params.max_object_size) {
                    num_objects++;
                }
            }
        }
        objects[num_objects].SumDistance += min_detections[i].Distance;
        objects[num_objects].SumIntensity += min_detections[i].Amplitude;
        objects[num_objects].SumAngleIntensity += (int32_t) i * min_detections[i].Amplitude;
        last_seg_distance = min_detections[i].Distance;
    }

    return num_objects;
}

static int8_t selectObject(const Object (&objects)[8], uint8_t num_objects,
                           const struct Track &tracked_object,
                           int32_t *selected_distance)
{
    int8_t best_match = 0;
    int32_t best_distance;
    uint32_t now = objects[best_match].Time;
    if(tracked_object.recent_update(now)) {
        // have a track, find the closest detection
        best_distance = tracked_object.distanceSq(objects[best_match]);
        for (uint8_t i = 1; i < num_objects; i++) {
            int32_t distance;
            distance = tracked_object.distanceSq(objects[i]);
            if (distance < best_distance) {
                best_distance = distance;
                best_match = i;
            }
        }
    } else {
        // no track, pick the nearest object
        best_distance = objects[best_match].radius();
        best_distance *= best_distance;
        for (uint8_t i = 1; i < num_objects; i++) {
            int32_t distance;
            distance = objects[i].radius();
            distance *= distance;
            if (distance < best_distance) {
                best_distance = distance;
                best_match = i;
            }
        }
    }
    *selected_distance = best_distance;
    return best_match;
}

static int8_t selectClosestObject(const Object (&objects)[8], uint8_t num_objects,
                           const struct Track &tracked_object,
                           int32_t *selected_distance)
{
    int8_t best_match = 0;
    int32_t best_distance;
    best_distance = objects[best_match].radius();
    best_distance *= best_distance;
    for (uint8_t i = 1; i < num_objects; i++)
    {
        int32_t distance;
        distance = objects[i].radius();
        distance *= distance;
        if (distance < best_distance)
        {
            best_distance = distance;
            best_match = i;
        }
    }
    uint32_t now = objects[best_match].Time;
    if(tracked_object.recent_update(now))
    {
        *selected_distance = tracked_object.distanceSq(objects[best_match]);
    }
    else
    {
        *selected_distance = best_distance;
    }
    return best_match;
}

void setObjectSegmentationParams(int16_t p_min_object_size,
                                 int16_t p_max_object_size,
                                 int16_t p_edge_call_threshold,
                                 bool p_closest_only)
{
    object_params.min_object_size    = p_min_object_size;
    object_params.max_object_size    = p_max_object_size;
    object_params.edge_call_threshold= p_edge_call_threshold;
    object_params.closest_only= p_closest_only;
    saveObjectSegmentationParameters();
}

static void saveObjectSegmentationParameters() {
    eeprom_write_block(&object_params, &saved_object_params,
                       sizeof(struct ObjectSegmentationParameters));
}


void restoreObjectSegmentationParameters() {
    eeprom_read_block(&object_params, &saved_object_params,
                       sizeof(struct ObjectSegmentationParameters));
}
