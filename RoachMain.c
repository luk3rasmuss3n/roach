/*
 * File: RoachMain.c
 *
 * Main for an ES_Framework project on the roach. Sets up the hardware, then
 * starts the framework. ES_Initialize() calls every registered service's Init
 * function (from ES_Configure.h), and ES_Run() is the forever loop that calls
 * the event checkers and delivers queued events to each service's Run function.
 */

#include "BOARD.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "roach.h"
#include <stdio.h>

void main(void)
{
    ES_Return_t ErrorType;

    BOARD_Init();      // chip setup: clocks, serial port (always first)
    Roach_Init();      // roach hardware: motors, bumpers, light sensor, LEDs

    printf("\r\nStarting ES Framework roach test");

    ErrorType = ES_Initialize();    // create queues, call every service's Init
    if (ErrorType == Success) {
        ErrorType = ES_Run();       // forever loop; only returns if something fails
    }

    // Only reached if initialization or the loop failed
    printf("\r\nES Framework stopped with error code %d", ErrorType);
    while (1) {
        ;
    }
}