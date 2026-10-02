/*
 * File: LightSubHSM.h
 * Minimal sub-state machine run inside RoachHSM's InLight state.
 * No Post function: a sub-machine has no queue; its parent passes it events.
 */
 
#ifndef LIGHTSUBHSM_H
#define LIGHTSUBHSM_H
 
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
 
uint8_t InitLightSubHSM(void);
ES_Event RunLightSubHSM(ES_Event ThisEvent);
 
#endif /* LIGHTSUBHSM_H */
 