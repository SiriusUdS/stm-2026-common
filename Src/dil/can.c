/**
  ******************************************************************************
  * @file    dil/can.c
  * @brief   STM32H7 FDCAN transport: RX filtering, interrupt-driven ring
  *          buffer and TX helper, built directly on the STM32H7 HAL.
  ******************************************************************************
  */

#include "dil/can.h"
#include <string.h>

/* Target id occupies bits [7:4] of the extended identifier */
#define TARGET_ID_SHIFT 4
#define TARGET_ID_MASK  (0xF << TARGET_ID_SHIFT)

#define FDCAN_RX_BUFFER_SIZE 32

typedef struct {
    FDCAN_RxHeaderTypeDef header;
    uint8_t data[8];
} FDCAN_RxMessageTypeDef;

typedef struct {
    FDCAN_RxMessageTypeDef messages[FDCAN_RX_BUFFER_SIZE];
    volatile uint16_t head;
    volatile uint16_t tail;
} FDCAN_RingBufferTypeDef;

/* Single-producer (ISR) / single-consumer (main loop) ring buffer */
static FDCAN_RingBufferTypeDef fdcanRxBuffer = { .head = 0, .tail = 0 };

/* Handle captured at init so CAN_Send() can reach the peripheral */
static FDCAN_HandleTypeDef *s_hfdcan = NULL;

bool CAN_Init(FDCAN_HandleTypeDef *hfdcan, uint8_t node_id)
{
  s_hfdcan = hfdcan;

  FDCAN_FilterTypeDef sFilterConfig;
  sFilterConfig.IdType = FDCAN_EXTENDED_ID;
  sFilterConfig.FilterIndex = 0;
  sFilterConfig.FilterType = FDCAN_FILTER_MASK;
  sFilterConfig.FilterConfig = FDCAN_FILTER_TO_RXFIFO0;
  sFilterConfig.FilterID1 = ((uint32_t)node_id << TARGET_ID_SHIFT);
  sFilterConfig.FilterID2 = TARGET_ID_MASK;

  if (HAL_FDCAN_ConfigFilter(s_hfdcan, &sFilterConfig) != HAL_OK) {
    return false;
  }

  if (HAL_FDCAN_ConfigGlobalFilter(
        s_hfdcan,
        FDCAN_REJECT,
        FDCAN_REJECT,
        FDCAN_REJECT_REMOTE,
        FDCAN_REJECT_REMOTE) != HAL_OK) {
    return false;
  }

  if (HAL_FDCAN_Start(s_hfdcan) != HAL_OK) {
    return false;
  }

  if (HAL_FDCAN_ActivateNotification(s_hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK) {
    return false;
  }

  return true;
}

bool CAN_Receive(CANHeader *outHeader, uint8_t *outData)
{
  if (fdcanRxBuffer.tail == fdcanRxBuffer.head) {
    return false;   // buffer empty
  }

  const FDCAN_RxMessageTypeDef *msg = &fdcanRxBuffer.messages[fdcanRxBuffer.tail];

  outHeader->code = msg->header.Identifier;
  memcpy(outData, msg->data, sizeof(msg->data));

  fdcanRxBuffer.tail = (fdcanRxBuffer.tail + 1) % FDCAN_RX_BUFFER_SIZE;
  return true;
}

bool CAN_Send(uint32_t extId, const uint8_t *data8)
{
  FDCAN_TxHeaderTypeDef txHeader = {0};
  txHeader.Identifier = extId;
  txHeader.IdType = FDCAN_EXTENDED_ID;
  txHeader.TxFrameType = FDCAN_DATA_FRAME;
  txHeader.DataLength = FDCAN_DLC_BYTES_8;
  txHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
  txHeader.BitRateSwitch = FDCAN_BRS_OFF;
  txHeader.FDFormat = FDCAN_CLASSIC_CAN;
  txHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
  txHeader.MessageMarker = 0;

  if (HAL_FDCAN_GetTxFifoFreeLevel(s_hfdcan) == 0) {
    return false;   // TX FIFO full
  }

  return HAL_FDCAN_AddMessageToTxFifoQ(s_hfdcan, &txHeader, (uint8_t *)data8) == HAL_OK;
}

/* FDCAN RX FIFO0 "new message" interrupt: producer side of the ring buffer */
void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if ((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != 0)
  {
    uint16_t next_head = (fdcanRxBuffer.head + 1) % FDCAN_RX_BUFFER_SIZE;

    if (next_head != fdcanRxBuffer.tail)
    {
      if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0,
                                 &fdcanRxBuffer.messages[fdcanRxBuffer.head].header,
                                 fdcanRxBuffer.messages[fdcanRxBuffer.head].data) == HAL_OK)
      {
        fdcanRxBuffer.head = next_head;
      }
    }
    else
    {
      // Ring buffer full: drain the hardware FIFO so it does not lock up
      FDCAN_RxHeaderTypeDef dummyHeader;
      uint8_t dummyData[8];
      HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &dummyHeader, dummyData);
    }
  }
}
