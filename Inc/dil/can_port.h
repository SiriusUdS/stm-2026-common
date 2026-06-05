#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "dil/can.h"   // can_handle_t

/* ------------------------------------------------------------------------- */
/* CAN port contract                                                         */
/*                                                                           */
/* These functions are DECLARED here but DEFINED by each consuming project   */
/* in its own adapter translation unit (e.g. dil/can_port_stm32h7.c). They   */
/* are bound at link time — there are no function pointers to initialise.    */
/* A project that forgets to provide one gets a link error.                  */
/*                                                                           */
/* The adapter is also responsible for delivering received frames to the     */
/* module by calling CAN_OnRxFrame() from its RX interrupt handler.          */
/* ------------------------------------------------------------------------- */

/**
 * @brief  Bring the CAN peripheral up: install an RX filter that accepts only
 *         frames addressed to @p node_id (rejecting everything else), start
 *         the peripheral and enable the RX "new message" interrupt.
 * @param  handle   Opaque peripheral handle passed through from CAN_Init().
 * @param  node_id  This node's identifier, matched against the frame target.
 * @retval true on success, false on any underlying error.
 */
bool can_port_init(can_handle_t handle, uint8_t node_id);

/**
 * @brief  Queue one classic extended-ID, 8-byte data frame for transmission.
 * @param  handle  Opaque peripheral handle passed through from CAN_Init().
 * @param  ext_id  29-bit extended identifier.
 * @param  data8   Pointer to the 8 payload bytes.
 * @retval true if the frame was queued, false if the TX FIFO was full or the
 *         underlying call failed.
 */
bool can_port_send(can_handle_t handle, uint32_t ext_id, const uint8_t *data8);
