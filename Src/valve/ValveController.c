#include "valve/ValveController.h"

void valveInit(Valve* valve, float expectedTransitTimeMs){
    servoInit(&(valve->servoConfig));
    servoSetPercent(&(valve->servoConfig), 0);

    valve->state = VALVE_STATE_CLOSED;
    valve->expectedTransitTimeMs = expectedTransitTimeMs;
}

void valveOpen(Valve* valve){
    if(valve->state == VALVE_STATE_OPEN) return;
    if(valve->state == VALVE_STATE_UNKNOWN) return;

    servoSetPercent(&(valve->servoConfig), 100);
    valve->state = VALVE_STATE_OPENING;
    valve->startTransitTimeMs = HAL_GetTick();
}

void valveClose(Valve* valve){
    if(valve->state == VALVE_STATE_CLOSING) return;
    if(valve->state == VALVE_STATE_UNKNOWN) return;

    servoSetPercent(&(valve->servoConfig), 0);
    valve->state = VALVE_STATE_CLOSING;
    valve->startTransitTimeMs = HAL_GetTick();
}

void valveUpdate(Valve* valve){
    if(valve->state == VALVE_STATE_CLOSING || valve->state == VALVE_STATE_OPENING){
        // Check si le temps est passé
        if((HAL_GetTick() - valve->startTransitTimeMs) >= valve->expectedTransitTimeMs){
            if(valve->state == VALVE_STATE_CLOSING) valve->state = VALVE_STATE_CLOSED;
            else valve->state = VALVE_STATE_OPEN;
        }
    }
}