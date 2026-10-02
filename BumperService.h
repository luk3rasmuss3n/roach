/*
 * File: BumperService.h
 * Public interface for the debounced bumper service (ES_Framework simple service).
 */

#ifndef BUMPERSERVICE_H
#define BUMPERSERVICE_H

#include "ES_Configure.h"
#include "ES_Framework.h"
#include "BOARD.h"

uint8_t InitBumperService(uint8_t Priority);
uint8_t PostBumperService(ES_Event ThisEvent);
ES_Event RunBumperService(ES_Event ThisEvent);

#endif /* BUMPERSERVICE_H */