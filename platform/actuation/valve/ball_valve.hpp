#pragma once

#include <cstddef>
#include <cstdint>

#include "stm32h7xx_hal.h"                   // GPIO_TypeDef + GPIO HAL API
#include "actuation/servomotor/hbl388.hpp"   // ServoConfig

/* ------------------------------------------------------------------------- *
 * Ball valve platform driver.
 *
 * Owns the servo, the limit switches and the transition/timeout state machine
 * for each valve, and DEFINES the logic-side seam declared in
 * actuation/interfaces/valve.hpp (logic::actuation::valve::open/close/
 * setOpenPercent/state).
 *
 * Only the HAL-coupled entry points (init, which needs the servo/GPIO config,
 * and tick, which samples the switches) are exposed here.
 * ------------------------------------------------------------------------- */

namespace platform::actuation::valve {

/**
 * @brief A limit switch wired to a GPIO input.
 */
struct LimitSwitchConfig {
    GPIO_TypeDef* port;  /**< GPIO port of the switch. */
    uint16_t      pin;   /**< GPIO pin of the switch. */
};

/**
 * @brief Static configuration of one ball valve.
 */
struct BallValveConfig {
    servomotor::ServoConfig servo;       /**< Servo actuating the ball. */
    LimitSwitchConfig open_limit;        /**< Switch asserted when fully open. */
    LimitSwitchConfig close_limit;       /**< Switch asserted when fully closed. */
    uint32_t max_transit_timeout_ms;     /**< Open/close time before declaring Fault. */
};

/**
 * @brief  Register the ball valves the platform exposes to the logic layer.
 *         Valve @c i is addressed as valveId @c i through the logic interface.
 * @param  configs  Caller-owned array of valve configurations.
 * @param  count    Number of valves in @p configs.
 */
void init(const BallValveConfig* configs, std::size_t count);

/**
 * @brief  Service every valve: debounce the limit switches and advance the
 *         opening/closing/timeout state machine. Call periodically from main().
 * @param  nowMs  Current millisecond tick (e.g. HAL_GetTick()).
 */
void tick(uint32_t nowMs);

} // namespace platform::actuation::valve
