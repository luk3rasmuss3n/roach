/*
 * File: RoachHelpers.h
 * Shared motor helpers and bumper masks, used by both RoachHSM and LightSubHSM.
 */

#ifndef ROACHHELPERS_H
#define ROACHHELPERS_H

#define DRIVE_SPEED 80                 // default motor speed (-100 to 100)

#define FRONT_LEFT_BUMPER   (1)        // bit 0: 0001
#define FRONT_RIGHT_BUMPER  (1 << 1)   // bit 1: 0010
#define REAR_LEFT_BUMPER    (1 << 2)   // bit 2: 0100
#define REAR_RIGHT_BUMPER   (1 << 3)   // bit 3: 1000

#define FRONT_BUMPERS (FRONT_LEFT_BUMPER | FRONT_RIGHT_BUMPER)   // 0011
#define REAR_BUMPERS  (REAR_LEFT_BUMPER | REAR_RIGHT_BUMPER)     // 1100

void MotorsStop(void);
void DriveForward(char speed);
void DriveBackward(char speed);
void DriveForwardLeft(char speed);
void DriveForwardRight(char speed);
void DriveBackLeft(char speed);
void DriveBackRight(char speed);
void DriveRight(char speed);
void DriveLeft(char speed);

#endif /* ROACHHELPERS_H */