#pragma once
#include "dil/can.h"

typedef void (*CANMsgHandler)(void *ctx,
                              const CANHeader *header,
                              const uint8_t *rxData);

typedef struct {
    uint32_t messageID;
    CANMsgHandler handler;
    void *ctx;
} CANHandlerEntry;

typedef struct {
    FDCAN_HandleTypeDef *hfdcan;
    CanNodeId nodeID;
    const CANHandlerEntry *handlers;
    uint8_t handlerCount;
} CANControllerConfig;