#ifndef CHUMP_TARGETING_H
#define CHUMP_TARGETING_H

#include <stdint.h>
#include "leddar_io.h"
#include "track.h"

uint8_t segmentObjects(const Detection (&min_detections)[LEDDAR_SEGMENTS],
                              uint32_t now,
                              Object (&objects)[8]);

int8_t trackObject(uint32_t now, struct Object (&objects)[8], uint8_t num_objects,
                   struct Track& tracked_object);

void setObjectSegmentationParams(int16_t p_min_object_size,
                                 int16_t p_max_object_size,
                                 int16_t p_edge_call_threshold,
                                 bool p_closest_only);

void restoreObjectSegmentationParameters(void);
#endif  // CHUMP_TARGETING_H
