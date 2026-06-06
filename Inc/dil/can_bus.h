#pragma once

/* ------------------------------------------------------------------------- *
 * CAN transport interface for the protocol layer (CANController + handlers)
 * and the application. This is the ONLY CAN header consumers should include;
 * the FDCAN driver in dil/can.h is an implementation detail reached through
 * can_bus.c.
 * ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"   /* FDCAN_HandleTypeDef */
#include "dil/can_types.h"   /* CANHeader */

/** Bind the FDCAN handle, install the RX filter for @p node_id, start the
 *  peripheral and enable the RX FIFO0 interrupt. Call once at startup. */
bool CanBus_Init(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id);

/** Queue a classic extended-ID, 8-byte frame for transmission. */
bool CanBus_Send(uint32_t extId, const uint8_t *data8);

/** Pop the oldest received frame. Returns false if none are available. */
bool CanBus_Receive(CANHeader *outHeader, uint8_t *outData);
