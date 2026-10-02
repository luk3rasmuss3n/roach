/*
 * File: RoachHelpers.c
 * Motor helper functions shared by the roach state machines.
 * Turns slow the INNER wheel so no wheel ever exceeds +/-100.
 */

#include "RoachHelpers.h"
#include "roach.h"

#define LAG_RATE 3    // turn sharpness: inner wheel = speed - speed/LAG_RATE (never 0)

void MotorsStop(void)
{
    Roach_LeftMtrSpeed(0);
    Roach_RightMtrSpeed(0);
}

void DriveForward(char speed)
{
    Roach_LeftMtrSpeed(speed);
    Roach_RightMtrSpeed(speed);
}

void DriveBackward(char speed)
{
    Roach_LeftMtrSpeed(-speed);
    Roach_RightMtrSpeed(-speed);
}

/* Forward, curving left: left (inner) wheel slower. */
void DriveForwardLeft(char speed)
{
    Roach_LeftMtrSpeed(speed - (speed / LAG_RATE));
    Roach_RightMtrSpeed(speed);
}

/* Forward, curving right: right (inner) wheel slower. */
void DriveForwardRight(char speed)
{
    Roach_LeftMtrSpeed(speed);
    Roach_RightMtrSpeed(speed - (speed / LAG_RATE));
}

void DriveRight(char speed)
{
    Roach_LeftMtrSpeed(speed);
    Roach_RightMtrSpeed(0);
}

void DriveLeft(char speed)
{
    Roach_LeftMtrSpeed(0);
    Roach_RightMtrSpeed(speed);
}

/* Backward, curving left: left (inner) wheel slower. */
void DriveBackLeft(char speed)
{
    Roach_LeftMtrSpeed(-(speed - (speed / LAG_RATE)));
    Roach_RightMtrSpeed(-speed);
}

/* Backward, curving right: right (inner) wheel slower. */
void DriveBackRight(char speed)
{
    Roach_LeftMtrSpeed(-speed);
    Roach_RightMtrSpeed(-(speed - (speed / LAG_RATE)));
}