#pragma once

#include <cstdint>

/* Telemetry representation of a single valve's state. */

/** @name Valve status flags (bitmask in ValveState::status) */
///@{
inline constexpr uint8_t SERVO_OPEN   = 1u << 0;
inline constexpr uint8_t SERVO_CLOSE  = 1u << 1;
inline constexpr uint8_t SERVO_MANUAL = 1u << 2;
///@}

/** @brief One valve's reported state. */
struct ValveState {
    uint8_t  status;    /**< Bitmask of SERVO_*. */
    uint8_t  reserved;  /**< Padding; keeps @ref value 2-byte aligned. */
    uint16_t value;     /**< Current position / setpoint. */
};

// Wire layout guard: must be packed with no implicit padding.
static_assert(sizeof(ValveState) == 4, "ValveState must be exactly 4 bytes");
static_assert(sizeof(ValveState) == sizeof(uint8_t)   // status
                                  + sizeof(uint8_t)   // reserved
                                  + sizeof(uint16_t), // value
              "ValveState has implicit padding — add explicit reserved bytes");
