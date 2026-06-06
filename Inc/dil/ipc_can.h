#pragma once

/* ------------------------------------------------------------------------- *
 * Inter-core CAN bridge (M7 <-> M4) over a shared, non-cacheable SRAM region.
 *
 * Reused by every STM32H747 board in the project (FillStation, Engine, ...):
 * the M4 owns FDCAN1 and runs IpcCan_M4Service(); the M7 runs the controller
 * and uses IpcCan_Send()/IpcCan_Receive(). Because this struct lives in the
 * shared library, all four binaries agree on the exact ring layout.
 *
 *   - M4: drains the TX ring onto the bus, publishes received frames to RX.
 *   - M7: queues frames into TX, consumes received frames from RX.
 *
 * Each ring is single-producer / single-consumer -> lock-free. The backing
 * struct is placed in the ".shared_ram" linker section (mapped to
 * IPC_SHARED_MPU_BASE on both cores, marked non-cacheable on the M7 via the
 * MPU), so plain volatile access + a __DMB() barrier is sufficient.
 * ------------------------------------------------------------------------- */

#include <stdint.h>
#include <stdbool.h>

#define IPC_CAN_TXQ_LEN  16u           /* must be a power of two */
#define IPC_CAN_RXQ_LEN  32u           /* must be a power of two */
#define IPC_CAN_MAGIC    0x43414E31u   /* 'CAN1' - init guard */

/* Base of the non-cacheable AXI-SRAM block holding the shared region (and the
 * SD/DMA buffers). Must match the SHARED region ORIGIN in every board's linker
 * script and the MPU region configured by IpcCan_MpuConfigShared(). */
#define IPC_SHARED_MPU_BASE  0x24040000u

typedef struct {
    uint32_t extId;     /* 29-bit extended identifier (CANHeader.code)      */
    uint8_t  data[8];   /* payload                                          */
    uint8_t  len;       /* valid payload length (always 8 for classic CAN)  */
} IpcCanFrame;

/* head is written only by the producer, tail only by the consumer.
 * Both are free-running counters; index = counter & (LEN-1).
 * empty: head == tail   full: (head - tail) == LEN                          */
typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    uint32_t          dropped;                 /* producer overflow counter  */
    IpcCanFrame       slot[IPC_CAN_TXQ_LEN];
} IpcTxRing;

typedef struct {
    volatile uint32_t head;
    volatile uint32_t tail;
    uint32_t          dropped;
    IpcCanFrame       slot[IPC_CAN_RXQ_LEN];
} IpcRxRing;

typedef struct {
    uint32_t  magic;    /* set to IPC_CAN_MAGIC by IpcCan_Init() on the M7   */
    IpcTxRing tx;       /* M7 -> M4  (M7 produces, M4 consumes)             */
    IpcRxRing rx;       /* M4 -> M7  (M4 produces, M7 consumes)             */
} IpcCanShared;

/* Single shared instance, placed at the start of the SHARED region by both
 * linker scripts so the two cores overlay the exact same bytes.            */
extern volatile IpcCanShared g_ipcCan;

/* ----- M7 (controller) API ----------------------------------------------- */

/** Zero the rings and arm the init guard. Call once on the M7 BEFORE the
 *  M4 is released from the boot-sync HSEM. */
void IpcCan_Init(void);

/** Queue a frame for the M4 to transmit. Returns false (and bumps the TX
 *  dropped counter) if the TX ring is full. */
bool IpcCan_Send(uint32_t extId, const uint8_t *data8);

/** Pop the oldest received frame. Returns false if none are available. */
bool IpcCan_Receive(uint32_t *extId, uint8_t *data8);

/** Configure the MPU region covering the shared/DMA AXI-SRAM block at
 *  IPC_SHARED_MPU_BASE (256 KB) as Normal/non-cacheable/shareable. Call from
 *  the M7's MPU setup, between HAL_MPU_Disable() and HAL_MPU_Enable(). */
void IpcCan_MpuConfigShared(uint8_t mpuRegionNumber);

/* ----- M4 (CAN service) API ---------------------------------------------- */

/** Pop the oldest frame the M7 queued for transmission. */
bool IpcCan_PopTx(IpcCanFrame *out);

/** Publish a frame received from the bus for the M7 to consume. Returns
 *  false (and bumps the RX dropped counter) if the RX ring is full. */
bool IpcCan_PushRx(uint32_t extId, const uint8_t *data8);

/** One-shot M4 service step: drain the TX ring onto the bus (CAN_Send) and
 *  publish any received frames into the RX ring (CAN_Receive). Call every
 *  iteration of the M4 main loop. Defined only for the M4 (CORE_CM4). */
void IpcCan_M4Service(void);
