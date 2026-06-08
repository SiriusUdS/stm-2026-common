#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

/* ------------------------------------------------------------------------- *
 * Statically-linked CAN interface for the logic layer.
 *
 * The logic layer depends ONLY on the declarations below. The platform layer
 * provides the definitions; they are resolved at link time. There is no
 * vtable and no HAL type in this header, so logic stays unaware of FDCAN, DMA,
 * or any board-specific detail.
 * ------------------------------------------------------------------------- */

namespace logic::communication {

/**
 * @brief Errors the CAN interface can report.
 */
enum class CanError {
    InternalError,  /**< Unspecified failure in the underlying transport. */
};

/** @brief Maximum number of payload bytes a classic CAN frame can carry. */
inline constexpr std::size_t MAX_PAYLOAD_LENGTH_BYTES = 8;

/**
 * @brief A hardware-agnostic CAN frame as seen by the logic layer.
 */
struct CanFrame {
    uint32_t id{};                                         /**< 29-bit extended identifier. */
    std::array<uint8_t, MAX_PAYLOAD_LENGTH_BYTES> data{};  /**< Payload */
    uint8_t length{};                                      /**< Valid bytes in @ref data (0..MAX_PAYLOAD_LENGTH_BYTES). */
};

namespace can {

/**
 * @brief  Queue a frame for transmission.
 * @param  frame  The frame to send.
 * @return std::nullopt on success, or a CanError describing the failure.
 */
[[nodiscard]] std::optional<CanError> send(const CanFrame& frame);

/**
 * @brief  Pop the oldest received frame.
 * @return The frame, or std::nullopt if none are available.
 */
[[nodiscard]] std::optional<CanFrame> receive();

} // namespace can

} // namespace logic::communication
