
/*
 * File: RoachHSM.h
 * Public interface for the top-level roach hierarchical state machine.
 */
 
#ifndef ROACHHSM_H
#define ROACHHSM_H
 
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
 
uint8_t InitRoachHSM(uint8_t Priority);
uint8_t PostRoachHSM(ES_Event ThisEvent);
ES_Event RunRoachHSM(ES_Event ThisEvent);
 
#endif /* ROACHHSM_H */
 