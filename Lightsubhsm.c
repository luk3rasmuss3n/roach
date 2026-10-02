/*
 * File: LightSubHSM.c
 * Based on TemplateSubHSM (ES_Framework, CMPE-118/L).
 *
 * Sub-state machine that runs INSIDE RoachHSM's InLight state.
 *
 *   Searching:  ENTRY          -> drive forward, start the 10 s JIG_PERIOD_TIMER
 *               BUMPER_PRESSED -> remember which bumper, go to BackingUp
 *               ES_TIMEOUT     -> (JIG_PERIOD_TIMER) go to Jigging
 *               EXIT           -> stop the period timer and motors
 *
 *   BackingUp:  ENTRY          -> move away from the bump (turning), start MOVE_TIMER
 *               ES_TIMEOUT     -> (MOVE_TIMER) back to Searching
 *               BUMPER_PRESSED -> bumped again: restart BackingUp
 *               EXIT           -> stop the timer and motors
 *
 *   Jigging:    ENTRY          -> first wiggle, start JIG_STEP_TIMER
 *               ES_TIMEOUT     -> (JIG_STEP_TIMER) next wiggle, or back to Searching when done
 *               BUMPER_PRESSED -> interrupt the jig, go to BackingUp
 *               EXIT           -> stop the step timer and motors
 *
 * Note: the 10 s timer restarts every time Searching is entered, so the roach
 * jigs after 10 seconds of UNINTERRUPTED searching (a bump resets the count).
 *
 * Events this machine handles are CONSUMED (changed to ES_NO_EVENT) so the
 * parent knows they were dealt with. Anything else (e.g. INTO_DARK) is returned
 * unchanged for the parent to handle. ES_ENTRY / ES_EXIT are never consumed.
 *
 * Requires in ES_Configure.h:
 *   #define TIMER1_RESP_FUNC PostRoachHSM     #define MOVE_TIMER       1
 *   #define TIMER2_RESP_FUNC PostRoachHSM     #define JIG_PERIOD_TIMER 2
 *   #define TIMER3_RESP_FUNC PostRoachHSM     #define JIG_STEP_TIMER   3
 */

/*******************************************************************************
 * MODULE #INCLUDE                                                             *
 ******************************************************************************/

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
#include "LightSubHSM.h"
#include "RoachHelpers.h"
#include <stdio.h>

/*******************************************************************************
 * MODULE #DEFINES                                                             *
 ******************************************************************************/

#define MOVE_TIME_MS     1000    // how long to react to a bump
 // 0.5 seconds move on bumper press
#define JIG_PERIOD_MS    10000   // jig after this long searching
#define JIG_STEP_MS      600    // how long each wiggle lasts
#define JIG_WIGGLES      2       // total wiggles per jig (alternating right/left)

/*******************************************************************************
 * PRIVATE TYPES AND VARIABLES                                                 *
 ******************************************************************************/

typedef enum {
    InitPSubState,
    Searching,
    BackingUp,
    Jigging,
} LightSubHSMState_t;

static const char *StateNames[] = {      // used by printf and by TattleTale (same order as enum)
    "InitPSubState",
    "Searching",
    "BackingUp",
    "Jigging",
};

static LightSubHSMState_t CurrentState = InitPSubState;
static uint16_t lastBump = 0;             // bumper bits from the most recent bump
static uint8_t jigStep = 0;               // which wiggle of the jig we're on

/*******************************************************************************
 * PRIVATE FUNCTION PROTOTYPES                                                 *
 ******************************************************************************/

static void Wiggle(uint8_t step);

/*******************************************************************************
 * PUBLIC FUNCTIONS                                                            *
 ******************************************************************************/

/**
 * @Function InitLightSubHSM(void)
 * @brief Called by the parent every time it ENTERS InLight. Resets to the
 *        pseudo-state and runs ES_INIT so the machine starts in Searching.
 *        No priority and no queue: a sub-machine only receives events that
 *        its parent hands it.
 */
uint8_t InitLightSubHSM(void)
{
    ES_Event returnEvent;

    CurrentState = InitPSubState;
    returnEvent = RunLightSubHSM(INIT_EVENT);
    if (returnEvent.EventType == ES_NO_EVENT) {
        return TRUE;
    }
    return FALSE;
}

/**
 * @Function RunLightSubHSM(ES_Event ThisEvent)
 * @brief Called by the parent with every event while in InLight. Returns
 *        ES_NO_EVENT if it handled the event, or the event unchanged if not.
 */
ES_Event RunLightSubHSM(ES_Event ThisEvent)
{
    uint8_t makeTransition = FALSE;
    LightSubHSMState_t nextState = CurrentState;

    //while(0){ES_Timer_InitTimer(JIG_PERIOD_TIMER, JIG_PERIOD_MS);   // jig in 10 s}
    

    

    switch (CurrentState) {
    case InitPSubState:
        if (ThisEvent.EventType == ES_INIT) {
            nextState = Jigging;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;           // consumed

        }
        break;


 
    case Searching:
          // jig in 10 s
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            //ES_Timer_InitTimer(JIG_PERIOD_TIMER, JIG_PERIOD_MS); 
            printf("\r\n  Sub: %s", StateNames[CurrentState]);
            DriveForward(DRIVE_SPEED);                   // look for dark
            
            break;
        case BUMPER_PRESSED:
            lastBump = ThisEvent.EventParam;             // remember which bumper
            nextState = BackingUp;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;           // consumed
            break;
        case ES_TIMEOUT:
            if (ThisEvent.EventParam == JIG_PERIOD_TIMER) {
                nextState = Jigging;                     // 10 s of searching: time to jig
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;       // consumed
            }
            break;
        case ES_EXIT:
        //    ES_Timer_StopTimer(JIG_PERIOD_TIMER);        // no stray "time to jig" later
            MotorsStop();
            break;
        default:
            break;                                       // not ours: return to parent
        }
        break;

    case BackingUp:
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            printf("\r\n  Sub: %s", StateNames[CurrentState]);
            if (lastBump & FRONT_BUMPERS) {
                DriveBackRight(DRIVE_SPEED);             // hit ahead: back away, turning
            } else {
                DriveForwardLeft(DRIVE_SPEED);           // hit from behind: pull away, turning
            }
            ES_Timer_InitTimer(MOVE_TIMER, MOVE_TIME_MS);
            break;
        case ES_TIMEOUT:
            if (ThisEvent.EventParam == MOVE_TIMER) {
                nextState = Searching;                   // done reacting
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;       // consumed
            }
            if (ThisEvent.EventParam == JIG_PERIOD_TIMER) {
                nextState = Jigging;                   // done reacting
                makeTransition = TRUE;
                ThisEvent.EventType = ES_NO_EVENT;       // consumed
            }

            break;
        case BUMPER_PRESSED:
            lastBump = ThisEvent.EventParam;             // bumped again mid-reaction
            nextState = BackingUp;                       // self-transition restarts it
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;           // consumed
            break;
        case ES_EXIT:
            ES_Timer_StopTimer(MOVE_TIMER);              // no stray timeout later
            MotorsStop();
            break;
        default:
            break;                                       // not ours: return to parent
        }
        break;

    case Jigging:
        switch (ThisEvent.EventType) {
        case ES_ENTRY:
            printf("\r\n  Sub: %s", StateNames[CurrentState]);
            jigStep = 0;                                 // start counting from the beginning
            Wiggle(jigStep);                             // first wiggle
            ES_Timer_InitTimer(JIG_STEP_TIMER, JIG_STEP_MS);
          //  ES_Timer_InitTimer(JIG_PERIOD_TIMER, JIG_PERIOD_MS); 
            break;
        case ES_TIMEOUT:
            if (ThisEvent.EventParam == JIG_STEP_TIMER) {
                jigStep++;
                if (jigStep < JIG_WIGGLES) {
                    Wiggle(jigStep);                     // next wiggle (other direction)
                    ES_Timer_InitTimer(JIG_STEP_TIMER, JIG_STEP_MS);
                } else {
                    nextState = Searching;               // jig finished
                    makeTransition = TRUE;
                }
             
                ThisEvent.EventType = ES_NO_EVENT;       // consumed
            }
            break;
        case BUMPER_PRESSED:
     //   ES_Timer_InitTimer(JIG_PERIOD_TIMER, JIG_PERIOD_MS);
            lastBump = ThisEvent.EventParam;             // bumped mid-jig: react instead
            nextState = BackingUp;
            makeTransition = TRUE;
            ThisEvent.EventType = ES_NO_EVENT;           // consumed
            break;
        case ES_EXIT:
        
            ES_Timer_StopTimer(JIG_STEP_TIMER);          // no stray step timeout later
            MotorsStop();
            ES_Timer_InitTimer(JIG_PERIOD_TIMER, JIG_PERIOD_MS);
            break;
        default:
            break;                                       // not ours: return to parent
        }
        break;

    default:
        break;
    }

    if (makeTransition == TRUE) {
        RunLightSubHSM(EXIT_EVENT);
        CurrentState = nextState;
        RunLightSubHSM(ENTRY_EVENT);
    }

 
    return ThisEvent;
}

/*******************************************************************************
 * PRIVATE FUNCTIONS                                                           *
 ******************************************************************************/

/* One wiggle of the jig: even steps curve right, odd steps curve left. */
static void Wiggle(uint8_t step)
{
    if (step % 2 == 0) {
        DriveRight(DRIVE_SPEED +20);
    } else {
        DriveLeft(DRIVE_SPEED + 20);
    }
}