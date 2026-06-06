/**
  ******************************************************************************
  * @file    dil/can_bus.c
  * @brief   Unified CAN transport for the protocol layer. Both cores now talk
  *          to the FDCAN peripheral directly through the dil/can.h driver; the
  *          inter-core IPC bridge has been retired.
  ******************************************************************************
  */

#include "dil/can_bus.h"
#include "dil/can.h"          /* FDCAN driver (CAN_Init / CAN_Send / CAN_Receive) */

bool CanBus_Init(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id)
{
    return CAN_Init(hfdcan, node_id);
}

bool CanBus_Send(uint32_t extId, const uint8_t *data8)
{
    return CAN_Send(extId, data8);
}

bool CanBus_Receive(CANHeader *outHeader, uint8_t *outData)
{
    return CAN_Receive(outHeader, outData);
}
