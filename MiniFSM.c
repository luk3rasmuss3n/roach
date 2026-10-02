/*
 * File: MiniFSM.c
 * Based on the TemplateFSM pattern for the ES_Framework (CMPE-118/L).
 *
 * A small test state machine that combines the light checker and the bumper service:
 *
 *   LightState: BUMPER_PRESSED  -> drive backward
 *               BUMPER_RELEASED -> stop
 *               INTO_DARK       -> go to DarkState
 *
 *   DarkState:  BUMPER_PRESSED  -> drive forward
 *               BUMPER_RELEASED -> stop
 *               INTO_LIGHT      -> go to LightState
 *
 * Starts in LightState, matching the light checker's startup assumption
 * (it posts INTO_DARK immediately if the roach powers on in the dark).
 * Motors stop whenever a state is exited.
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "BOARD.h"
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "MiniFSM.h"
#include "roach.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define DRIVE_SPEED 80    // motor speed while moving (-100 to 100); tune from Part 4 tests
#define FRONT_LEFT_BUMPER   (1)        // bit 0: 0001
#define FRONT_RIGHT_BUMPER  (1 << 1)   // bit 1: 0010
#define REAR_LEFT_BUMPER    (1 << 2)   // bit 2: 0100
#define REAR_RIGHT_BUMPER   (1 << 3)   // bit 3: 1000
#define LAG_RATE    2   
#define MOVE_TIME_MS 1000
#define MOTOR_OFFSET 0.9

/*******************************************************************************
 * PRIVATE TYPES AND VARIABLES                                                 *
 ******************************************************************************/

typedef enum {
    InitPState,     // initial pseudo-state: only waits for ES_INIT
    LightState,
    DarkState,
} MiniFSMState_t;

static const char *StateNames[] = {
    "InitPState",
    "LightState",
    "DarkState",
};



static MiniFSMState_t CurrentState = InitPState;
static uint8_t MyPriority;

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/

static void Drive(char speed);
static void DriveForward(char speed);
static void DriveBackward(char speed);
static void DriveForwardLeft(char speed);
static void DriveForwardRight(char speed);
static void DriveBackLeft(char speed);
static void DriveBackRight(char speed);

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitMiniFSM(uint8_t Priority)
 * @brief Called once by the framework. Saves the priority, resets the state,
 *        and posts ES_INIT to this machine's own queue.
 */
uint8_t InitMiniFSM(uint8_t Priority)
{
    ES_Event ThisEvent;

    MyPriority = Priority;
    CurrentState = InitPState;

    ThisEvent.EventType = ES_INIT;
    if (ES_PostToService(MyPriority, ThisEvent) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostMiniFSM(ES_Event ThisEvent)
 * @brief Puts an event into this machine's queue. The light checker and the
 *        bumper service post here.
 */
uint8_t PostMiniFSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunMiniFSM(ES_Event ThisEvent)
 * @brief Called by the framework with one event at a time. Switches on the
 *        current state first, then on the event.
 */
ES_Event RunMiniFSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE;
    MiniFSMState_t nextState = CurrentState;
    ES_Event EntryEvent = {ES_ENTRY, 0};
    ES_Event ExitEvent = {ES_EXIT, 0};

    switch (CurrentState) {
    case InitPState:
        if (ThisEvent.EventType == ES_INIT) {
            nextState = LightState;              // assume light at startup
            makeTransition = TRUE;
        }
        break;

    case LightState:
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            Drive(0);
            printf("\r\nState: %s", StateNames[CurrentState]);
            DriveForward(DRIVE_SPEED);
            break;
        case BUMPER_PRESSED:
            if (ThisEvent.EventParam & FRONT_LEFT_BUMPER || ThisEvent.EventParam & FRONT_RIGHT_BUMPER) {   // was front left involved? and ThisEvent.EventParam is the mask
                //turnfaster
                DriveBackRight(DRIVE_SPEED);
            }
            else if (ThisEvent.EventParam & REAR_LEFT_BUMPER || ThisEvent.EventParam & REAR_RIGHT_BUMPER) {   // was front left involved? and ThisEvent.EventParam is the mask
                //turnfaster
                DriveForwardLeft(DRIVE_SPEED);
            }
            ES_Timer_InitTimer(MOVE_TIMER, MOVE_TIME_MS);   // move for this long
            break;

            

        case ES_TIMEOUT:
            if (ThisEvent.EventParam == MOVE_TIMER) {       // only react to OUR timer
                Drive(0); 
                nextState = LightState;     
                makeTransition = TRUE;        
            }
            break;

        case INTO_DARK:
            nextState = DarkState;
            makeTransition = TRUE;
            break;
        case ES_EXIT:
            Drive(0);                            // never carry motion into the next state
            break;
        default:
            break;
        }
        break;
    

    case DarkState:
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            Drive(0);
            printf("\r\nState: %s", StateNames[CurrentState]);

            break;
        // case BUMPER_PRESSED:
        //     if (ThisEvent.EventParam & FRONT_LEFT_BUMPER || ThisEvent.EventParam & FRONT_RIGHT_BUMPER) {   // was front left involved? and ThisEvent.EventParam is the mask
        //         DriveBackward(DRIVE_SPEED);
        //     }
        //     else if (ThisEvent.EventParam & REAR_LEFT_BUMPER || ThisEvent.EventParam & REAR_RIGHT_BUMPER) {
        //         DriveForward(DRIVE_SPEED);
        //     }
        //     break;
        // case BUMPER_RELEASED: //make this on a timer instead
        //     Drive(0);
        //     break;
        case INTO_LIGHT:
            nextState = LightState;
            makeTransition = TRUE;
            break;
        case ES_EXIT:
            Drive(0);                            // never carry motion into the next state
            break;
        default:
            break;
        }
        break;
    }

    if (makeTransition == TRUE) {
        RunMiniFSM(ExitEvent);       // let the old state clean up
        CurrentState = nextState;
        RunMiniFSM(EntryEvent);      // let the new state set up
    }

    ThisEvent.EventType = ES_NO_EVENT;
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

/* Drive both wheels at the same speed: positive = forward, negative = backward. */
static void DriveForward(char speed)
{
    Roach_LeftMtrSpeed(speed*MOTOR_OFFSET);
    Roach_RightMtrSpeed(speed);
}

static void DriveBackward(char speed)
{
    Roach_LeftMtrSpeed(-speed*MOTOR_OFFSET);
    Roach_RightMtrSpeed(-speed);
}



//turns :)!
static void DriveForwardLeft(char speed)
{
    Roach_LeftMtrSpeed(speed - (speed / LAG_RATE));
    Roach_RightMtrSpeed(speed);
}
 
/* Forward, curving right: right (inner) wheel slower. */
static void DriveForwardRight(char speed)
{
    Roach_LeftMtrSpeed(speed);
    Roach_RightMtrSpeed(speed - (speed / LAG_RATE));
}
 
/* Backward, curving left: left (inner) wheel slower. */
static void DriveBackLeft(char speed)
{
    Roach_LeftMtrSpeed(-(speed - (speed / LAG_RATE)));
    Roach_RightMtrSpeed(-speed);
}
 
/* Backward, curving right: right (inner) wheel slower. */
static void DriveBackRight(char speed)
{
    Roach_LeftMtrSpeed(-speed);
    Roach_RightMtrSpeed(-(speed - (speed / LAG_RATE)));
}

static void Drive(char speed){
    Roach_LeftMtrSpeed(speed*MOTOR_OFFSET);
    Roach_RightMtrSpeed(speed);
}