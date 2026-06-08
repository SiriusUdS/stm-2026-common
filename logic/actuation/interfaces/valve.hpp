#pragma once

#include <cstdint>
#include <optional>

/* ------------------------------------------------------------------------- *
 * Statically-linked valve interface for the logic layer.
 *
 * The logic layer depends ONLY on the declarations below. The platform layer
 * provides the definitions (servo PWM, limit switches, transition timeout);
 * they are resolved at link time. There is no vtable and no HAL type here, so
 * logic commands valves without knowing about servos, timers or GPIO.
 * ------------------------------------------------------------------------- */

namespace logic::actuation {

/**
 * @brief Observable state of a valve.
 */
enum class ValveState {
    Unknown,  /**< State not yet determined. */
    Open,     /**< Fully open (open limit reached). */
    Closed,   /**< Fully closed (close limit reached). */
    Opening,  /**< Transitioning toward open. */
    Closing,  /**< Transitioning toward closed. */
    Fault,    /**< Transition timed out or limit switches inconsistent. */
};

/**
 * @brief Errors the valve interface can report.
 */
enum class ValveError {
    InternalError,  /**< Unspecified failure, or no valve with the given id. */
};

namespace valve {

/**
 * @brief  Command a valve to move to its fully-open position.
 * @param  valve_id  Identifier of the valve to actuate.
 * @return std::nullopt on success, or a ValveError describing the failure.
 */
[[nodiscard]] std::optional<ValveError> open(uint8_t valve_id);

/**
 * @brief  Command a valve to move to its fully-closed position.
 * @param  valve_id  Identifier of the valve to actuate.
 * @return std::nullopt on success, or a ValveError describing the failure.
 */
[[nodiscard]] std::optional<ValveError> close(uint8_t valve_id);

/**
 * @brief  Command a valve to a proportional open position.
 * @param  valve_id  Identifier of the valve to actuate.
 * @param  percent  Desired opening, 0 (fully closed) to 100 (fully open).
 *                  Values outside the range are clamped.
 * @return std::nullopt on success, or a ValveError describing the failure.
 */
[[nodiscard]] std::optional<ValveError> setOpenPercent(uint8_t valve_id, float percent);

/**
 * @brief  Read the latest known state of a valve.
 * @param  valve_id  Identifier of the valve to query.
 * @return The valve's current state, or ValveState::Unknown for an invalid id.
 */
[[nodiscard]] ValveState state(uint8_t valve_id);

} // namespace valve

} // namespace logic::actuation
