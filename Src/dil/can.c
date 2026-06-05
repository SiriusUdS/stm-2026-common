/**
  ******************************************************************************
  * @file    dil/can.c
  * @brief   HAL-independent CAN transport: software RX ring buffer and the
  *          public API. All hardware access is delegated to the project's
  *          port layer (dil/can_port.h), bound at link time.
  ******************************************************************************
  */

#include "dil/can.h"
#include "dil/can_port.h"
#include <string.h>

#define CAN_RX_BUFFER_SIZE 32

typedef struct {
    can_frame_t frames[CAN_RX_BUFFER_SIZE];
    volatile uint16_t head;   // written by CAN_OnRxFrame (producer / ISR)
    volatile uint16_t tail;   // written by CAN_Receive    (consumer / main)
} CanRingBufferTypeDef;

/* Single-producer (ISR) / single-consumer (main loop) ring buffer */
static CanRingBufferTypeDef rxBuffer = { .head = 0, .tail = 0 };

/* Handle captured at init so CAN_Send() can reach the peripheral */
static can_handle_t s_handle = NULL;

bool CAN_Init(can_handle_t handle, uint8_t node_id)
{
  s_handle = handle;
  return can_port_init(s_handle, node_id);
}

bool CAN_Receive(CANHeader *outHeader, uint8_t *outData)
{
  if (rxBuffer.tail == rxBuffer.head) {
    return false;   // buffer empty
  }

  const can_frame_t *frame = &rxBuffer.frames[rxBuffer.tail];

  outHeader->code = frame->id;
  memcpy(outData, frame->data, sizeof(frame->data));

  rxBuffer.tail = (rxBuffer.tail + 1) % CAN_RX_BUFFER_SIZE;
  return true;
}

bool CAN_Send(uint32_t extId, const uint8_t *data8)
{
  return can_port_send(s_handle, extId, data8);
}

void CAN_OnRxFrame(const can_frame_t *frame)
{
  uint16_t next_head = (rxBuffer.head + 1) % CAN_RX_BUFFER_SIZE;

  if (next_head == rxBuffer.tail) {
    return;   // ring buffer full: drop the frame
  }

  rxBuffer.frames[rxBuffer.head] = *frame;
  rxBuffer.head = next_head;
}
