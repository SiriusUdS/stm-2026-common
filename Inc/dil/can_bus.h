#pragma once

/* ------------------------------------------------------------------------- *
 * Unified CAN transport for the protocol layer (CANController + handlers).
 *
 * The same protocol code runs on either core; the transport is resolved at
 * compile time in can_bus.c:
 *   - M4 (CORE_CM4): the FDCAN driver in dil/can.h (CAN_Send / CAN_Receive).
 *   - M7 (otherwise): the inter-core IPC bridge in dil/ipc_can.h
 *                     (IpcCan_Send / IpcCan_Receive).
 *
 * This lets the FillStation/Engine boards keep the FDCAN peripheral on the M4
 * while running CANController on the M7, talking over the shared-SRAM rings.
 * ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include "dil/can_types.h"   /* CANHeader (HAL-free) */

/** Queue a classic extended-ID, 8-byte frame for transmission. */
bool CanBus_Send(uint32_t extId, const uint8_t *data8);

/** Pop the oldest received frame. Returns false if none are available. */
bool CanBus_Receive(CANHeader *outHeader, uint8_t *outData);
