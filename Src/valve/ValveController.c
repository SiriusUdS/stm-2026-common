#include "valve/ValveController.h"

extern uint32_t HAL_GetTick(void);

#define DEBOUNCE_DELAY_MS 30

void valveInit(Valve* const valve, uint32_t timeoutMs) {
    if (valve == NULL) return; // Critical C guard: protect against null pointers

    valve->state = VALVE_STATE_UNKNOWN;
    valve->maxTransitTimeoutMs = timeoutMs;
    valve->startTransitionMs = 0;
    valve->lastLimitSwitchMs = 0;
    valve->openLimitHit = false;
    valve->closeLimitHit = false;
}

void valveOpen(Valve* const valve) {
    if (valve == NULL) return;

    if (valve->state == VALVE_STATE_OPEN || valve->state == VALVE_STATE_OPENING) {
        return;
    }
    
    valve->startTransitionMs = HAL_GetTick();
    valve->state = VALVE_STATE_OPENING;
    
    servoSetPercent(&(valve->servoConfig), 100);
}

void valveClose(Valve* const valve) {
    if (valve == NULL) return;

    if (valve->state == VALVE_STATE_CLOSED || valve->state == VALVE_STATE_CLOSING) {
        return;
    }

    valve->startTransitionMs = HAL_GetTick();
    valve->state = VALVE_STATE_CLOSING;

    servoSetPercent(&(valve->servoConfig), 0);
}

void valveUpdate(Valve* const valve, bool rawOpenLimitPin, bool rawCloseLimitPin) {
    if (valve == NULL) return;

    uint32_t currentMs = HAL_GetTick();

    // Debounce
    if (rawOpenLimitPin != valve->openLimitHit || rawCloseLimitPin != valve->closeLimitHit) {
        if ((currentMs - valve->lastLimitSwitchMs) >= DEBOUNCE_DELAY_MS) {
            valve->openLimitHit = rawOpenLimitPin;
            valve->closeLimitHit = rawCloseLimitPin;
            valve->lastLimitSwitchMs = currentMs;
        }
    }

    switch (valve->state) {
        case VALVE_STATE_OPENING:
            if (valve->openLimitHit) {
                valve->state = VALVE_STATE_OPEN;
            } 
            else if ((currentMs - valve->startTransitionMs) >= valve->maxTransitTimeoutMs) {
                valve->state = VALVE_STATE_FAULT;
            }
            break;

        case VALVE_STATE_CLOSING:
            if (valve->closeLimitHit) {
                valve->state = VALVE_STATE_CLOSED;
            } 
            else if ((currentMs - valve->startTransitionMs) >= valve->maxTransitTimeoutMs) {
                valve->state = VALVE_STATE_FAULT;
            }
            break;

        default:
            break;
    }
}