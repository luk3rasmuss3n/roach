
/*
 * File: MiniFSM.h
 * Public interface for the mini light/bumper test state machine.
 */
 
#ifndef MINIFSM_H
#define MINIFSM_H
 
#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"
 
uint8_t InitMiniFSM(uint8_t Priority);
uint8_t PostMiniFSM(ES_Event ThisEvent);
ES_Event RunMiniFSM(ES_Event ThisEvent);

 
#endif /* MINIFSM_H */
 