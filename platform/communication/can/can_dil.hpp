#pragma once

#include <cstdint>
#include <optional>

#include "stm32h7xx_hal.h"                    // FDCAN_HandleTypeDef + FDCAN HAL API
#include "communication/interfaces/can.hpp"   // logic::communication::CanError

/* ------------------------------------------------------------------------- *
 * Platform CAN DIL (Driver Interface Layer).
 *
 * Owns the STM32H7 FDCAN peripheral: RX filtering, the interrupt-driven RX
 * ring buffer and the TX path. It also DEFINES the logic-side seam declared in
 * communication/interfaces/can.hpp (logic::communication::can::send/receive),
 * so the logic layer reaches CAN without ever seeing a HAL type.
 *
 * Only the HAL-coupled entry point (init, which needs the FDCAN handle) is
 * exposed here; send/receive belong to the logic interface.
 * ------------------------------------------------------------------------- */

namespace platform::communication::can {

/**
 * @brief  Bind the FDCAN peripheral, install the RX acceptance filter for this
 *         node, start the peripheral and enable the RX FIFO0 interrupt.
 * @param  hfdcan  FDCAN peripheral handle (e.g. &hfdcan1), already created by
 *                 the CubeMX init.
 * @param  nodeId  This node's identifier, matched against the target-id field
 *                 of incoming extended identifiers.
 * @return std::nullopt on success, or a CanError describing the failure.
 */
[[nodiscard]] std::optional<logic::communication::CanError>
init(FDCAN_HandleTypeDef* hfdcan, uint8_t nodeId);

} // namespace platform::communication::can
