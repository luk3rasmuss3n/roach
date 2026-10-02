/*
 * File: RoachHSM.c
 * Based on TemplateHSM (ES_Framework, CMPE-118/L).
 *
 * Top-level hierarchical state machine for the roach:
 *
 *   InLight:  contains LightSubHSM (Searching <-> BackingUp)
 *             ENTRY      -> (re)start the sub-machine in Searching
 *             all other events go to the sub-machine FIRST;
 *             INTO_DARK  -> go to InDark   (written once, applies to every sub-state)
 *
 *   InDark:   ENTRY           -> stop (hide)
 *             BUMPER_PRESSED  -> front: back up / rear: go forward
 *             BUMPER_RELEASED -> stop
 *             INTO_LIGHT      -> go to InLight
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "RoachHSM.h"
#include "LightSubHSM.h"      // the sub-machine run inside InLight
#include "RoachHelpers.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINE                                                             *
 ******************************************************************************/
#define DARK_MOVE_TIME_MS 500   
 
/*******************************************************************************
 * PRIVATE TYPES AND VARIABLES                                                 *
 ******************************************************************************/

typedef enum {
    InitPState,
    InLight,
    InDark,
} RoachHSMState_t;

static const char *StateNames[] = {      // used by printf and by TattleTale
    "InitPState",
    "InLight",
    "InDark",
};

static RoachHSMState_t CurrentState = InitPState;
static uint8_t MyPriority;

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitRoachHSM(uint8_t Priority)
 * @brief Called once by the framework. Saves the priority and posts ES_INIT.
 */
uint8_t InitRoachHSM(uint8_t Priority)
{
    MyPriority = Priority;
    CurrentState = InitPState;
    if (ES_PostToService(MyPriority, INIT_EVENT) == TRUE) {
        return TRUE;
    } else {
        return FALSE;
    }
}

/**
 * @Function PostRoachHSM(ES_Event ThisEvent)
 * @brief The light checker, bumper service, and MOVE_TIMER all post here.
 *        (The sub-machine has no queue; it gets events through this machine.)
 */
uint8_t PostRoachHSM(ES_Event ThisEvent)
{
    return ES_PostToService(MyPriority, ThisEvent);
}

/**
 * @Function RunRoachHSM(ES_Event ThisEvent)
 * @brief Called by the framework with one event at a time.
 */
ES_Event RunRoachHSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE;
    RoachHSMState_t nextState = CurrentState;



    switch (CurrentState) {
    case InitPState:
        if (ThisEvent.EventType == ES_INIT) {
            nextState = InLight;                         // assume light at startup
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
        }
        break;

    case InLight:
        if (ThisEvent.EventType == ES_ENTRY) {
            printf("\r\nState: %s", StateNames[CurrentState]);
            InitLightSubHSM();                           // always start fresh in Searching
        } else {
            ThisEvent = RunLightSubHSM(ThisEvent);       // sub-machine gets FIRST look
        }

        switch (ThisEvent.EventType) {                   // whatever the sub-machine didn't consume
        case INTO_DARK:                                  // written ONCE for every sub-state
            nextState = InDark;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        default:
            break;
        }
        break;

    case InDark:
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            printf("\r\nState: %s", StateNames[CurrentState]);
            MotorsStop();                                // found dark: hide
            break;
        case BUMPER_PRESSED:
            if (ThisEvent.EventParam & FRONT_BUMPERS) {
                DriveBackward(DRIVE_SPEED);
            } else if (ThisEvent.EventParam & REAR_BUMPERS) {
                DriveForward(DRIVE_SPEED);
            }
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case BUMPER_RELEASED:
            ES_Timer_InitTimer(MOVE_TIMER, DARK_MOVE_TIME_MS);
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case INTO_LIGHT:
            nextState = InLight;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;
            break;
        case ES_TIMEOUT:
            if (ThisEvent.EventParam == MOVE_TIMER) {       // only react to OUR timer
                MotorsStop();
               // nextState = DarkState;     
               // makeTransition = TRUE;    
               ThisEvent.EventType = ES_NO_EVENT;    
            }
            break;
        case ES_EXIT:
            MotorsStop();
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }

    if (makeTransition == TRUE) {
        RunRoachHSM(EXIT_EVENT);     // in InLight, this EXIT also reaches the sub-machine
        CurrentState = nextState;
        RunRoachHSM(ENTRY_EVENT);
    }

 
    return ThisEvent;
}