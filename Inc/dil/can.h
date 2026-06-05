#pragma once

#include <stdint.h>
#include <stdbool.h>

/* ------------------------------------------------------------------------- */
/* Neutral types — this module is independent of any vendor HAL.             */
/* The concrete peripheral handle is provided by the project's port layer    */
/* (see dil/can_port.h) and is treated here as an opaque pointer.            */
/* ------------------------------------------------------------------------- */
typedef void *can_handle_t;

typedef struct {
    uint32_t id;        // 29-bit extended identifier
    uint8_t  data[8];   // classic CAN payload
} can_frame_t;

/* ------------------------------------------------------------------------- */
/* CAN extended-identifier layout (29-bit), overlaid on a 32-bit word        */
/* ------------------------------------------------------------------------- */
typedef struct {
    uint32_t senderID:4;
    uint32_t targetID:4;
    uint32_t deviceState:4;
    uint32_t messageID:8;
    uint32_t errorCtrl:2;
    uint32_t errorCode:7;
    uint32_t reserved:3;
}FrameCANHeader;

typedef union {
    FrameCANHeader frame;
    uint32_t code;
} CANHeader;

// Identifie les nodes du réseau CAN
typedef enum {
    CAN_NODE_FILL_F412   = 0x01,
    CAN_NODE_ENGINE_H747 = 0x02,
} CanNodeId;

// Id du message
typedef enum {
    CAN_ID_CMD_VALVE      = 0x01,   /* FILL  → ENGINE : Change valve status   */
    CAN_ID_STATUS_VALVE   = 0x02,   /* ENGINE → FILL  : Return valve status   */
} CanMsgId;

// Index des valves
typedef enum {
    CAN_VALVE_1 = 1,
    CAN_VALVE_2 = 2,
} CanValveIndex;

// Type de commande (FILL  → ENGINE)
typedef enum {
    CAN_CMD_CLOSE = 0x00,
    CAN_CMD_OPEN  = 0x01,
} CanValveCmd;

// Status de la valve
typedef enum {
    CAN_STATUS_UNKNOWN  = 0x00,
    CAN_STATUS_OPEN     = 0x01,
    CAN_STATUS_OPENING  = 0x02,
    CAN_STATUS_CLOSED    = 0x03,
    CAN_STATUS_CLOSING  = 0x04,
} CanValveStatus;

/* ------------------------------------------------------------------------- */
/* Public API                                                                */
/* ------------------------------------------------------------------------- */

/**
 * @brief  Store the peripheral handle and bring the CAN port up so that only
 *         frames addressed to @p node_id are accepted (delegates to
 *         can_port_init()). Call once after the peripheral has been created.
 * @param  handle   Opaque peripheral handle (e.g. &hfdcan1).
 * @param  node_id  This node's identifier (CanNodeId), used as the RX filter.
 * @retval true on success, false if the port failed to initialise.
 */
bool CAN_Init(can_handle_t handle, uint8_t node_id);

/**
 * @brief  Pop the oldest received frame from the software RX ring buffer.
 * @param  outHeader  Filled with the decoded 29-bit extended identifier.
 * @param  outData    Buffer of at least 8 bytes, filled with the payload.
 * @retval true   a frame was dequeued, false if the buffer was empty.
 */
bool CAN_Receive(CANHeader *outHeader, uint8_t *outData);

/**
 * @brief  Queue a classic extended-ID, 8-byte data frame for transmission
 *         (delegates to can_port_send()).
 * @param  extId  29-bit extended identifier.
 * @param  data8  Pointer to the 8 payload bytes.
 * @retval true   the frame was queued, false if the TX FIFO was full or the
 *                port call failed.
 */
bool CAN_Send(uint32_t extId, const uint8_t *data8);

/**
 * @brief  Push a freshly received frame into the RX ring buffer. Intended to
 *         be called from the project's RX interrupt adapter. ISR-safe
 *         (single-producer). Frames are dropped if the buffer is full.
 * @param  frame  The decoded frame to enqueue.
 */
void CAN_OnRxFrame(const can_frame_t *frame);
