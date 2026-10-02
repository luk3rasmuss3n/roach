/*
 * File: roachEventChecker.h
 * Public interface for the roach event checkers. ES_Configure.h points at this
 * file through EVENT_CHECK_HEADER, so the framework can see every checker
 * listed in EVENT_CHECK_LIST.
 */
 
#ifndef ROACHEVENTCHECKER_H
#define ROACHEVENTCHECKER_H
 
#include "ES_Configure.h"
#include "BOARD.h"
 
uint8_t CheckLightupdated(void);   // light checker with hysteresis: INTO_LIGHT / INTO_DARK
 
#endif /* ROACHEVENTCHECKER_H */
 