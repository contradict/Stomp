#ifndef DRIVE_H
#define DRIVE_H

void initDrive();

// parameter passed by reference. This routine clamps the value to the maximum
// range of -1000 to 1000, passing a reference ensures that the clamped value
// is stored in the history.

void drive(int16_t &p_speed);

void driveTelem(void);

#endif // DRIVE_H
