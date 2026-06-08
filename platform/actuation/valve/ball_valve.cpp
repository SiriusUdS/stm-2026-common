/**
  ******************************************************************************
  * @file    actuation/valve/ball_valve.cpp
  * @brief   Ball valve driver and platform-side definition of the logic valve
  *          interface. Owns the servo, limit switches and transition/timeout
  *          state machine for each valve.
  ******************************************************************************
  */

#include "actuation/valve/ball_valve.hpp"
#include "actuation/interfaces/valve.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

using logic::actuation::ValveError;
using logic::actuation::ValveState;

namespace servo = platform::actuation::servomotor;
namespace pv    = platform::actuation::valve;

namespace {

constexpr std::size_t MAX_VALVE_COUNT  = 8;
constexpr uint32_t    DEBOUNCE_DELAY_MS = 30;
constexpr float       MIN_OPEN_PERCENT = 0.0F;
constexpr float       MAX_OPEN_PERCENT = 100.0F;

struct ValveRuntime {
    pv::BallValveConfig config{};
    ValveState state               = ValveState::Unknown;
    float      open_percent        = 0.0F;   /* last commanded opening */
    uint32_t   start_transition_ms = 0;
    uint32_t   last_limit_switch_ms = 0;
    bool       open_limit_hit      = false;
    bool       close_limit_hit     = false;
};

std::array<ValveRuntime, MAX_VALVE_COUNT> s_valves{};
std::size_t s_valve_count = 0;

/* Map a desired opening percentage to a servo travel percentage.
   TODO: a ball valve's flow vs. ball rotation is non-linear; replace this
   linear pass-through with the real curve (coefficients or a lookup table). */
float open_percent_to_servo_percent(float open_percent)
{
    return open_percent;  // linear stub
}

bool read_limit(const pv::LimitSwitchConfig& limit)
{
    return HAL_GPIO_ReadPin(limit.port, limit.pin) == GPIO_PIN_SET;
}

} // namespace

/* -------------------------------------------------------------------------- */
/* Platform entry points                                                      */
/* -------------------------------------------------------------------------- */

namespace platform::actuation::valve {

void init(const BallValveConfig* configs, std::size_t count)
{
    s_valve_count = (count > MAX_VALVE_COUNT) ? MAX_VALVE_COUNT : count;
    for (std::size_t i = 0; i < s_valve_count; ++i) {
        s_valves[i] = ValveRuntime{};
        s_valves[i].config = configs[i];
        servo::init(configs[i].servo);
    }
}

void tick(uint32_t now_ms)
{
    for (std::size_t i = 0; i < s_valve_count; ++i) {
        ValveRuntime& valve = s_valves[i];

        const bool raw_open  = read_limit(valve.config.open_limit);
        const bool raw_close = read_limit(valve.config.close_limit);

        // Debounce limit switch transitions.
        if (raw_open != valve.open_limit_hit || raw_close != valve.close_limit_hit) {
            if ((now_ms - valve.last_limit_switch_ms) >= DEBOUNCE_DELAY_MS) {
                valve.open_limit_hit  = raw_open;
                valve.close_limit_hit = raw_close;
                valve.last_limit_switch_ms = now_ms;
            }
        }

        switch (valve.state) {
            case ValveState::Opening:
                if (valve.open_limit_hit) {
                    valve.state = ValveState::Open;
                } else if ((now_ms - valve.start_transition_ms) >= valve.config.max_transit_timeout_ms) {
                    valve.state = ValveState::Fault;
                }
                break;
            case ValveState::Closing:
                if (valve.close_limit_hit) {
                    valve.state = ValveState::Closed;
                } else if ((now_ms - valve.start_transition_ms) >= valve.config.max_transit_timeout_ms) {
                    valve.state = ValveState::Fault;
                }
                break;
            default:
                break;
        }
    }
}

} // namespace platform::actuation::valve

/* -------------------------------------------------------------------------- */
/* Logic-side seam: definitions for actuation/interfaces/valve.hpp            */
/* -------------------------------------------------------------------------- */

namespace logic::actuation::valve {

std::optional<ValveError> open(uint8_t valve_id)
{
    if (valve_id >= s_valve_count) return ValveError::InternalError;
    ValveRuntime& valve = s_valves[valve_id];

    if (valve.state == ValveState::Open || valve.state == ValveState::Opening) {
        return std::nullopt;
    }
    valve.start_transition_ms = HAL_GetTick();
    valve.state        = ValveState::Opening;
    valve.open_percent = MAX_OPEN_PERCENT;
    servo::setPercent(valve.config.servo, open_percent_to_servo_percent(MAX_OPEN_PERCENT));
    return std::nullopt;
}

std::optional<ValveError> close(uint8_t valve_id)
{
    if (valve_id >= s_valve_count) return ValveError::InternalError;
    ValveRuntime& valve = s_valves[valve_id];

    if (valve.state == ValveState::Closed || valve.state == ValveState::Closing) {
        return std::nullopt;
    }
    valve.start_transition_ms = HAL_GetTick();
    valve.state        = ValveState::Closing;
    valve.open_percent = MIN_OPEN_PERCENT;
    servo::setPercent(valve.config.servo, open_percent_to_servo_percent(MIN_OPEN_PERCENT));
    return std::nullopt;
}

std::optional<ValveError> setOpenPercent(uint8_t valve_id, float percent)
{
    if (valve_id >= s_valve_count) return ValveError::InternalError;
    ValveRuntime& valve = s_valves[valve_id];

    if (percent < MIN_OPEN_PERCENT) percent = MIN_OPEN_PERCENT;
    if (percent > MAX_OPEN_PERCENT) percent = MAX_OPEN_PERCENT;

    valve.open_percent = percent;
    // TODO: decide how state() should reflect an intermediate hold; we likely
    // keep open_percent alongside the state rather than forcing Opening/Closing.
    servo::setPercent(valve.config.servo, open_percent_to_servo_percent(percent));
    return std::nullopt;
}

ValveState state(uint8_t valve_id)
{
    if (valve_id >= s_valve_count) return ValveState::Unknown;
    return s_valves[valve_id].state;
}

} // namespace logic::actuation::valve
