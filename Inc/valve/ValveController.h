#pragma once
#include "dil/ServoDriver.h"

typedef enum {
    VALVE_STATE_UNKNOWN,
    VALVE_STATE_OPEN,
    VALVE_STATE_CLOSED,
    VALVE_STATE_OPENING,
    VALVE_STATE_CLOSING
} ValveState;

typedef struct {
    ServoConfig servoConfig;
    ValveState state;
    float startTransitTimeMs;
    float expectedTransitTimeMs;
} Valve;

void valveInit(Valve* valve, float expectedTransitTimeMs);
void valveOpen(Valve* valve);
void valveClose(Valve* valve);
void valveUpdate(Valve* valve);