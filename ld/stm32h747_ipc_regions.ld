/* ========================================================================= *
 * Shared inter-core IPC + DMA memory regions (STM32H747 dual-core).
 *
 * INCLUDE this file inside the MEMORY {} block of EVERY CM7 and CM4 linker
 * script in the project (FillStation, Engine, ...). It guarantees that the IPC
 * transport shipped in stm-2026-common (dil/ipc_can.c: the `.shared_ram`
 * section at IPC_SHARED_MPU_BASE, and IpcCan_MpuConfigShared()) links and runs
 * identically on every board.
 *
 * Resolution: the build adds `-L <this directory>` to the linker flags, so the
 * INCLUDE below uses a bare filename.
 *
 * ---- D1 AXI-SRAM (0x24000000, 512K) partition contract --------------------
 *   0x24000000 .. 0x24040000   256K   CM4 private RAM   (declared per-core in
 *                                      the CM4 script; NOT defined here)
 *   0x24040000 .. 0x24070000   192K   SHARED  - g_ipcCan + SPI4(DMA1) buffers,
 *                                      shared M7<->M4 (this chip only)
 *   0x24070000 .. 0x24080000    64K   SD_RAM  - SDMMC/FATFS DMA buffers
 *
 * The CM7 MPU marks 0x24040000..0x24080000 (256K) Normal/non-cacheable/
 * shareable via IpcCan_MpuConfigShared(); keep these two regions contiguous so
 * that single MPU region covers both.
 *
 * NOTE: this is per-chip shared memory (each board's own M7<->M4). Boards talk
 * to each other over the CAN bus, not this RAM - so the addresses only need to
 * be consistent between the two cores of one chip, which this file enforces.
 * ========================================================================= */

SHARED (xrw) : ORIGIN = 0x24040000, LENGTH = 192K
SD_RAM (xrw) : ORIGIN = 0x24070000, LENGTH = 64K
