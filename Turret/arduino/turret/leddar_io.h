#ifndef LEDDAR_IO_H
#define LEDDAR_IO_H

#include <stdint.h>
#include <stddef.h>

#define LEDDAR_FREQ 50
#define LEDDAR_SEGMENTS 16

// Represents a measurement
struct Detection
{
  static const uint16_t reset_distance=10000;
  uint8_t Segment;
  int16_t Distance;
  int16_t Amplitude;

  // Default constructor
  Detection() : Segment(0), Distance(reset_distance), Amplitude(0) { }
  // reset distance/amplitude to defaults
  void reset(void) { Distance = reset_distance; Amplitude = 0; };
};

void leddarWrapperInit();

void requestDetections();
bool bufferDetections();
uint8_t parseDetections();
void calculateMinimumDetections(size_t good_detections);

size_t getRawDetections(const Detection **detections);
size_t getMinimumDetections(const Detection (**detections)[LEDDAR_SEGMENTS]);

void setLeddarParameters(int16_t min_object_distance,
                         int16_t max_object_distance);
 
#endif  // LEDDAR_IO_H
