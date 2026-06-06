#include "dil/ServoDriver.h"

void servoSetPulseWidth(ServoConfig* servo, int width){
    __HAL_TIM_SET_COMPARE(servo->htim, servo->channel, width);
}
void servoSetPercent(ServoConfig* servo, float percent){
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;

    int width = servo->minPulseMs + (percent/100.0f) * (servo->maxPulseMs-servo->minPulseMs);

    servoSetPulseWidth(servo, width);
}
void servoInit(ServoConfig* servo){
    HAL_TIM_PWM_Start(servo->htim, servo->channel);
}
