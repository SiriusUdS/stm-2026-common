/* ========================================================================= *
 * Output sections for the shared inter-core IPC + DMA regions.
 *
 * INCLUDE this file inside the SECTIONS {} block of EVERY CM7 and CM4 linker
 * script, after the core-local sections. Pairs with stm32h747_ipc_regions.ld.
 *
 *   .shared_ram -> SHARED : holds g_ipcCan (dil/ipc_can.c) and any cross-core
 *                  buffers placed with __attribute__((section(".shared_ram"))).
 *   .sd_ram     -> SD_RAM : SDMMC/FATFS DMA buffers placed with
 *                  __attribute__((section(".sd_ram"))). Empty (and harmless) on
 *                  a core/board that does not use SD.
 *
 * Both are NOLOAD: startup never initialises them, so g_ipcCan keeps whatever
 * IpcCan_Init() wrote and is not clobbered by the C runtime.
 * ========================================================================= */

.shared_ram (NOLOAD) :
{
  . = ALIGN(4);
  KEEP(*(.shared_ram))
  . = ALIGN(4);
} >SHARED

.sd_ram (NOLOAD) :
{
  . = ALIGN(4);
  KEEP(*(.sd_ram))
  . = ALIGN(4);
} >SD_RAM
