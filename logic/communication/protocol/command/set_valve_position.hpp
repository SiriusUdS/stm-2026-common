#pragma once

#include <cstdint>

/* Payload for CommandType::SetValvePosition — drive a valve to a position. */

/** @name Valve actions (bitmask in SetValvePositionFrame::action) */
///@{
inline constexpr uint8_t VALVE_OPEN   = 1u << 0;
inline constexpr uint8_t VALVE_CLOSE  = 1u << 1;
inline constexpr uint8_t VALVE_MANUAL = 1u << 2;
///@}

/** @brief SetValvePosition payload: requested action and target position. */
struct SetValvePositionFrame {
    uint8_t  action;    /**< Bitmask of VALVE_*. */
    uint8_t  reserved;  /**< Padding; keeps @ref value 2-byte aligned. */
    uint16_t value;     /**< Target position (e.g. servo setpoint). */
};
