#pragma once
#include "stm32h7xx_hal.h"

typedef struct {
    TIM_HandleTypeDef *htim;
    uint32_t channel;
    float minPulseMs;
    float maxPulseMs;
} ServoConfig;

void servoSetPulseWidth(ServoConfig* servo, int width);
void servoSetPercent(ServoConfig* servo, float percent);
void servoInit(ServoConfig* servo);
