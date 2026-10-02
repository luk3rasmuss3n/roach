/*
 * File: BumperService.c
 * Based on TemplateService.c by J. Edward Carryer, modified by Gabriel H Elkaim
 *
 * Simple service for the ES_Framework that samples the roach bumpers every
 * BUMPER_SAMPLE_MS milliseconds (driven by a framework timer) and debounces them.
 * A new bumper reading is only accepted after it has stayed the same for
 * DEBOUNCE_COUNT extra samples. When an accepted reading changes, the service
 * posts BUMPER_PRESSED or BUMPER_RELEASED, with the 4-bit bumper value in
 * EventParam:
 *
 *   bit 3: rear right   bit 2: rear left   bit 1: front right   bit 0: front left
 *
 * Test harness: define SIMPLESERVICE_TEST in the project to have the service
 * post its events to itself and print them.
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "BOARD.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BumperService.h"
#include "roach.h"
#include <stdio.h>


/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define BUMPER_SAMPLE_MS  5    // 5 ms between samples = 200 Hz
#define DEBOUNCE_COUNT    4    // extra matching samples needed (~20 ms stable)

/*******************************************************************************
 * PRIVATE MODULE VARIABLES                                                    *
 ******************************************************************************/

static uint8_t MyPriority;             // this service's queue ID, set by the framework
static uint8_t debouncedBumpers = 0;   // last ACCEPTED reading (none pressed at start)
static uint8_t lastSample = 0;         // previous raw sample
static uint8_t stableCount = 0;        // how many samples in a row matched lastSample

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitBumperService(uint8_t Priority)
 * @param Priority - which event queue this service uses (assigned by the framework)
 * @return TRUE or FALSE
 * @brief Called once by the framework during ES_Initialize(). Saves the priority
 *        and posts ES_INIT to this service's own queue.
 */
uint8_t InitBumperService(uint8_t Priority)
{
    ES_Event ThisEvent;

    MyPriority = Priority;

    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostBumperService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to be posted to this service's queue
 * @return TRUE or FALSE
 * @brief Wrapper around the framework's posting function. This name is used in
 *        ES_Configure.h (e.g. TIMER0_RESP_FUNC) to route events to this service.
 */
uint8_t PostBumperService(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunBumperService(ES_Event ThisEvent)
 * @param ThisEvent - the event (type and param) to respond to
 * @return ES_NO_EVENT if handled without errors
 * @brief Called by the framework whenever this service's queue has an event.
 *        ES_INIT starts the sample timer. Each ES_TIMEOUT takes one bumper sample,
 *        updates the debounce logic, posts an event on a real change, and restarts
 *        the timer.
 */
ES_Event RunBumperService(ES_Event ThisEvent)
{
    ES_Event ReturnEvent;
    ES_Event PostEvent;                              // the event we send out
    uint8_t sample;                                  // this tick's bumper reading

    ReturnEvent.EventType = ES_NO_EVENT;             // assume no errors

    switch (ThisEvent.EventType) {
    case ES_INIT:
        ES_Timer_InitTimer(BUMPER_TIMER, BUMPER_SAMPLE_MS);   // start the heartbeat
        break;

    case ES_TIMEOUT:
        if (ThisEvent.EventParam != BUMPER_TIMER) {  // only react to OUR timer
            break;
        }

        sample = Roach_ReadBumpers();                // 1. one sample

        if (sample == lastSample) {                  // 2. same as last sample?
            if (stableCount < DEBOUNCE_COUNT) {
                stableCount++;                       //    yes: count it
            }
        } else {
            stableCount = 0;                         //    no: moved, start over
            lastSample = sample;
        }

        if ((stableCount >= DEBOUNCE_COUNT) &&      // 3. stable long enough...
            (sample != debouncedBumpers)) {         //    ...and a real change?
            if (sample & ~debouncedBumpers) {        // 4. newly pressed bits?
                PostEvent.EventType = BUMPER_PRESSED;
            } else {
                PostEvent.EventType = BUMPER_RELEASED;
            }
            PostEvent.EventParam = sample;           //    which bumpers, as bits
            debouncedBumpers = sample;               // 5. accept it
#ifndef SIMPLESERVICE_TEST
            PostRoachHSM(PostEvent); 
#else
            PostBumperService(PostEvent);            //    test: send to myself to print
#endif
        }

        ES_Timer_InitTimer(BUMPER_TIMER, BUMPER_SAMPLE_MS);   // 6. restart: ALWAYS
        break;

#ifdef SIMPLESERVICE_TEST
    default:                                         // test: print anything else we receive
        printf("\r\nEvent: %s\tParam: 0x%X",
                EventNames[ThisEvent.EventType], ThisEvent.EventParam);
        break;
#endif
    }

    return ReturnEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/