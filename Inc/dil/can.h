#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "stm32h7xx_hal.h"   // FDCAN_HandleTypeDef and FDCAN HAL API
#include "dil/can_types.h"   // shared, HAL-free CAN protocol types + enums

/* ------------------------------------------------------------------------- */
/* Public API (FDCAN driver - M4 only, requires the FDCAN HAL module)        */
/* ------------------------------------------------------------------------- */

/**
 * @brief  Store the FDCAN handle, install an RX filter that accepts only
 *         frames addressed to @p node_id, start the peripheral and enable the
 *         RX FIFO0 interrupt. Call once after the peripheral has been created.
 * @param  hfdcan   FDCAN peripheral handle (e.g. &hfdcan1).
 * @param  node_id  This node's identifier (CanNodeId), used as the RX filter.
 * @retval true on success, false if any HAL call failed.
 */
bool CAN_Init(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id);

/**
 * @brief  Pop the oldest received frame from the software RX ring buffer.
 * @param  outHeader  Filled with the decoded 29-bit extended identifier.
 * @param  outData    Buffer of at least 8 bytes, filled with the payload.
 * @retval true   a frame was dequeued, false if the buffer was empty.
 */
bool CAN_Receive(CANHeader *outHeader, uint8_t *outData);

/**
 * @brief  Queue a classic extended-ID, 8-byte data frame for transmission.
 * @param  extId  29-bit extended identifier.
 * @param  data8  Pointer to the 8 payload bytes.
 * @retval true   the frame was queued, false if the TX FIFO was full or the
 *                HAL call failed.
 */
bool CAN_Send(uint32_t extId, const uint8_t *data8);
