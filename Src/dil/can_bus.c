/**
  ******************************************************************************
  * @file    dil/can_bus.c
  * @brief   Per-core resolution of the unified CAN transport used by the
  *          protocol layer. M4 maps onto the FDCAN driver, M7 onto the
  *          inter-core IPC bridge.
  ******************************************************************************
  */

#include "dil/can_bus.h"

#if defined(CORE_CM4)

#include "dil/can.h"          /* FDCAN driver, M4 only */

bool CanBus_Send(uint32_t extId, const uint8_t *data8)
{
    return CAN_Send(extId, data8);
}

bool CanBus_Receive(CANHeader *outHeader, uint8_t *outData)
{
    return CAN_Receive(outHeader, outData);
}

#else /* M7: protocol over the inter-core bridge */

#include "dil/ipc_can.h"

bool CanBus_Send(uint32_t extId, const uint8_t *data8)
{
    return IpcCan_Send(extId, data8);
}

bool CanBus_Receive(CANHeader *outHeader, uint8_t *outData)
{
    return IpcCan_Receive(&outHeader->code, outData);
}

#endif
