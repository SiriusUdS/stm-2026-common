#pragma once
#include <stdint.h>



typedef struct {
    uint8_t forceState:1;
    uint8_t confirmSync:1;
    uint8_t syncTime:1;
    uint8_t diagnose:1;
    uint8_t reserved:4;
} FrameSyncFlags;


typedef union {
    FrameSyncFlags frame;
    uint8_t code;
} SyncFlags;

typedef struct {
    uint8_t deviceIDFrom;
    uint8_t deviceIDTo;
    uint8_t syncState;
    SyncFlags syncFlags;
} FrameSyncPacket;

/// @brief This packet sync the desired device with the state from another. Every devices on the network MUST treat this packet.
typedef union {
    FrameSyncPacket frame;
    uint8_t data[sizeof(FrameSyncPacket)];
} SyncPacket;