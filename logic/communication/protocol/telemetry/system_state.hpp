#pragma once

#include <cstdint>

#include "sirius-headers-common/Telecommunication/InterfaceField.h"
#include "communication/protocol/telemetry/valve_state.hpp"

/* Periodic downlink telemetry: the board's full live state sent to the ground
 * station — timestamps, interface flags, every ADC channel and valve state. */

struct SystemState {
    uint32_t       frameTs_MS;          /**< Time this frame was assembled. */
    uint32_t       lastHandshakeTs_MS;  /**< Time of the last GS handshake. */
    InterfaceField interfaces;
    uint32_t       raw_adc_values[12];
    ValveState     valve_states[2];
};

// Wire layout guard: the downlink telemetry must be packed with no implicit
// padding so the ground station decodes it byte-for-byte. If a field change
// introduces a gap, this fails — add an explicit reserved field to close it.
static_assert(sizeof(SystemState) == 2 * sizeof(uint32_t)    // frameTs_MS + lastHandshakeTs_MS
                                   + sizeof(InterfaceField)   // interfaces
                                   + 12 * sizeof(uint32_t)    // raw_adc_values
                                   + 2 * sizeof(ValveState),  // valve_states
              "SystemState has implicit padding — add explicit reserved bytes");
