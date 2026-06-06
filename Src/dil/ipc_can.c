#include "dil/ipc_can.h"
#include "stm32h7xx_hal.h"   /* __DMB(), MPU API */

#if defined(CORE_CM4)
#include "dil/can.h"         /* CAN_Send / CAN_Receive (FDCAN driver, M4 only) */
#endif

#if defined(__GNUC__)
#define IPC_SHARED_SECTION __attribute__((section(".shared_ram")))
#else
#define IPC_SHARED_SECTION
#endif

/* Lives in the SHARED region (IPC_SHARED_MPU_BASE). Declared identically on
 * both cores; NOLOAD in the linker scripts so startup never zeroes it. */
IPC_SHARED_SECTION volatile IpcCanShared g_ipcCan;

static inline void ipc_copy8_in(volatile uint8_t *dst, const uint8_t *src)
{
    for (uint32_t i = 0u; i < 8u; i++) { dst[i] = src[i]; }
}

static inline void ipc_copy8_out(uint8_t *dst, volatile const uint8_t *src)
{
    for (uint32_t i = 0u; i < 8u; i++) { dst[i] = src[i]; }
}

void IpcCan_Init(void)
{
    g_ipcCan.tx.head = 0u; g_ipcCan.tx.tail = 0u; g_ipcCan.tx.dropped = 0u;
    g_ipcCan.rx.head = 0u; g_ipcCan.rx.tail = 0u; g_ipcCan.rx.dropped = 0u;
    __DMB();
    g_ipcCan.magic = IPC_CAN_MAGIC;
    __DMB();
}

/* --- TX ring: M7 producer, M4 consumer ----------------------------------- */

bool IpcCan_Send(uint32_t extId, const uint8_t *data8)
{
    uint32_t h = g_ipcCan.tx.head;
    uint32_t t = g_ipcCan.tx.tail;
    if ((h - t) >= IPC_CAN_TXQ_LEN) { g_ipcCan.tx.dropped++; return false; }

    volatile IpcCanFrame *s = &g_ipcCan.tx.slot[h & (IPC_CAN_TXQ_LEN - 1u)];
    s->extId = extId;
    ipc_copy8_in(s->data, data8);
    s->len = 8u;
    __DMB();                       /* publish payload before advancing head */
    g_ipcCan.tx.head = h + 1u;
    return true;
}

bool IpcCan_PopTx(IpcCanFrame *out)
{
    uint32_t h = g_ipcCan.tx.head;
    uint32_t t = g_ipcCan.tx.tail;
    if (h == t) { return false; }  /* empty */

    volatile IpcCanFrame *s = &g_ipcCan.tx.slot[t & (IPC_CAN_TXQ_LEN - 1u)];
    out->extId = s->extId;
    ipc_copy8_out(out->data, s->data);
    out->len = s->len;
    __DMB();                       /* consume payload before advancing tail */
    g_ipcCan.tx.tail = t + 1u;
    return true;
}

/* --- RX ring: M4 producer, M7 consumer ----------------------------------- */

bool IpcCan_PushRx(uint32_t extId, const uint8_t *data8)
{
    uint32_t h = g_ipcCan.rx.head;
    uint32_t t = g_ipcCan.rx.tail;
    if ((h - t) >= IPC_CAN_RXQ_LEN) { g_ipcCan.rx.dropped++; return false; }

    volatile IpcCanFrame *s = &g_ipcCan.rx.slot[h & (IPC_CAN_RXQ_LEN - 1u)];
    s->extId = extId;
    ipc_copy8_in(s->data, data8);
    s->len = 8u;
    __DMB();
    g_ipcCan.rx.head = h + 1u;
    return true;
}

bool IpcCan_Receive(uint32_t *extId, uint8_t *data8)
{
    uint32_t h = g_ipcCan.rx.head;
    uint32_t t = g_ipcCan.rx.tail;
    if (h == t) { return false; }  /* empty */

    volatile IpcCanFrame *s = &g_ipcCan.rx.slot[t & (IPC_CAN_RXQ_LEN - 1u)];
    *extId = s->extId;
    ipc_copy8_out(data8, s->data);
    __DMB();
    g_ipcCan.rx.tail = t + 1u;
    return true;
}

/* --- M7 MPU helper (shared so every board configures it identically) ------ */

void IpcCan_MpuConfigShared(uint8_t mpuRegionNumber)
{
    MPU_Region_InitTypeDef MPU_InitStruct = {0};

    MPU_InitStruct.Enable           = MPU_REGION_ENABLE;
    MPU_InitStruct.Number           = mpuRegionNumber;
    MPU_InitStruct.BaseAddress      = IPC_SHARED_MPU_BASE;
    MPU_InitStruct.Size             = MPU_REGION_SIZE_256KB;  /* SHARED (192K) + SD_RAM (64K) */
    MPU_InitStruct.SubRegionDisable = 0x0;
    MPU_InitStruct.TypeExtField     = MPU_TEX_LEVEL1;         /* Normal, non-cacheable */
    MPU_InitStruct.AccessPermission = MPU_REGION_FULL_ACCESS;
    MPU_InitStruct.DisableExec      = MPU_INSTRUCTION_ACCESS_DISABLE;
    MPU_InitStruct.IsShareable      = MPU_ACCESS_SHAREABLE;
    MPU_InitStruct.IsCacheable      = MPU_ACCESS_NOT_CACHEABLE;
    MPU_InitStruct.IsBufferable     = MPU_ACCESS_NOT_BUFFERABLE;

    HAL_MPU_ConfigRegion(&MPU_InitStruct);
}

/* --- M4 service step (FDCAN driver, M4 only) ------------------------------ */

#if defined(CORE_CM4)
void IpcCan_M4Service(void)
{
    /* TX: frames queued by the M7 -> CAN bus */
    IpcCanFrame txf;
    while (IpcCan_PopTx(&txf))
    {
        CAN_Send(txf.extId, txf.data);
    }

    /* RX: frames from the bus -> shared ring for the M7 */
    CANHeader rxHeader;
    uint8_t   rxData[8];
    while (CAN_Receive(&rxHeader, rxData))
    {
        IpcCan_PushRx(rxHeader.code, rxData);
    }
}
#endif /* CORE_CM4 */
