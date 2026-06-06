#pragma once
#include "dil/ServoDriver.h"
#include <stdbool.h>

typedef enum {
    VALVE_STATE_UNKNOWN,
    VALVE_STATE_OPEN,
    VALVE_STATE_CLOSED,
    VALVE_STATE_OPENING,
    VALVE_STATE_CLOSING,
    VALVE_STATE_FAULT
} ValveState;

typedef struct {
    GPIO_TypeDef* port;
    uint16_t      pin;
} LimitSwitchConfig;

typedef struct {
    ServoConfig servoConfig;
    
    LimitSwitchConfig openLimitSwitch;
    LimitSwitchConfig closeLimitSwitch;
    
    uint32_t startTransitionMs;
    uint32_t lastLimitSwitchMs;
    uint32_t maxTransitTimeoutMs;
    
    ValveState state;
    bool openLimitHit;
    bool closeLimitHit;
} Valve;

void valveInit(Valve* const valve, uint32_t timeoutMs);
void valveOpen(Valve* const valve);
void valveClose(Valve* const valve);
void valveUpdate(Valve* const valve, bool rawOpenLimitPin, bool rawCloseLimitPin);