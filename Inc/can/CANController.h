#pragma once
#include "can/CANControllerConfig.h"

typedef struct {
    const CANControllerConfig *cfg;
} CANController;

void CANController_Init (CANController *ctrl, const CANControllerConfig *cfg);
void CANController_Process(CANController *ctrl);