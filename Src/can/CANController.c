#include "can/CANController.h"

void CANController_Init(CANController *ctrl, const CANControllerConfig *cfg)
{
    ctrl->cfg = cfg;
}

void CANController_Process(CANController *ctrl)
{
    CANHeader header;
    uint8_t rxData[8];

    if (!CAN_Receive(&header, rxData)) return;
    if (header.frame.targetID != ctrl->cfg->nodeID) return;

    const CANHandlerEntry *h = ctrl->cfg->handlers;
    for (uint8_t i = 0; i < ctrl->cfg->handlerCount; i++, h++)
    {
        if (h->messageID == header.frame.messageID)
        {
            h->handler(h->ctx, &header, rxData);
            return;
        }
    }
}