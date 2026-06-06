# Shared STM32H747 linker template (inter-core IPC + DMA)

These files keep every board's linker scripts in agreement with the shared
inter-core CAN transport in `stm-2026-common` (`dil/ipc_can.c`). They define the
memory regions and output sections that **must be identical across both cores of
a chip** so that:

- `g_ipcCan` (placed in `.shared_ram`) overlays the same bytes on the M7 and M4,
- `IpcCan_MpuConfigShared()` and the linker agree on the shared/DMA address range.

| File | Goes inside | Defines |
|------|-------------|---------|
| `stm32h747_ipc_regions.ld`  | `MEMORY { }`   | `SHARED`, `SD_RAM` regions |
| `stm32h747_ipc_sections.ld` | `SECTIONS { }` | `.shared_ram`, `.sd_ram` sections |

> This is **per-chip** shared memory (a board's own M7 ↔ M4). Boards talk to each
> other over the **CAN bus**, not this RAM — so the addresses only need to be
> consistent between the two cores of one chip, which these files enforce.

## D1 AXI-SRAM (`0x24000000`, 512K) partition contract

```
0x24000000 .. 0x24040000   256K   CM4 private RAM   (per-core, in the CM4 script)
0x24040000 .. 0x24070000   192K   SHARED  - g_ipcCan + SPI4 (DMA1) buffers
0x24070000 .. 0x24080000    64K   SD_RAM  - SDMMC / FATFS DMA buffers
```

The CM7 MPU marks `0x24040000..0x24080000` (256K) Normal / non-cacheable /
shareable via `IpcCan_MpuConfigShared(MPU_REGION_NUMBER1)`. `SHARED` and `SD_RAM`
are kept contiguous so a single MPU region covers both. This matches
`IPC_SHARED_MPU_BASE` in `dil/ipc_can.h`.

Place buffers with:
```c
__attribute__((section(".shared_ram"))) /* cross-core, e.g. SPI4 DMA */
__attribute__((section(".sd_ram")))     /* SDMMC / FATFS DMA buffers */
```

## What the template does NOT define (per-board, per-core)

Each `.ld` still declares its own core-local memory, which legitimately differs:

| | M7 RAM | M4 RAM | D2 SRAM `0x30000000` |
|---|--------|--------|----------------------|
| FillStation | DTCM 128K | AXI-SRAM 256K | 288K → Ethernet |
| Engine      | DTCM 128K | AXI-SRAM 256K | 288K → free (no ETH) |

SPI6 buffers must stay in D3 SRAM4 (`0x38000000`, `RAM_D3` / `.SRAM4`) — BDMA
cannot reach AXI-SRAM.

## Porting a board (e.g. Engine: no Ethernet, has SD)

1. Point the board's `stm-2026-common` submodule at the commit carrying this
   `ld/` directory plus `dil/ipc_can.*`, `dil/can_types.h`, `dil/can.h`.
2. In **both** `CM7.ld` and `CM4.ld`:
   - inside `MEMORY { }`:  `INCLUDE stm32h747_ipc_regions.ld`
   - inside `SECTIONS { }`: `INCLUDE stm32h747_ipc_sections.ld`
3. In **both** `CM7/CMakeLists.txt` and `CM4/CMakeLists.txt`, add the search path
   so the bare-filename `INCLUDE`s resolve:
   ```cmake
   set(STM32_LINKER_OPTION "-L${CMAKE_CURRENT_LIST_DIR}/../stm-2026-common/ld")
   ```
4. M7 `MPU_Config()`: call `IpcCan_MpuConfigShared(MPU_REGION_NUMBER1)` between
   `HAL_MPU_Disable()` and `HAL_MPU_Enable()`.
5. M7 `main()`: call `IpcCan_Init()` before releasing the M4 (before the boot-sync
   HSEM release). M4 main loop: call `IpcCan_M4Service()` each iteration after
   `CAN_Init(&hfdcanX, <THIS_BOARD_NODE_ID>)`.

### Engine specifics
- `CM7.ld`: **omit** `RAM_D2` and the `.RxDescripSection` / `.TxDescripSection` /
  `.RxBuffSection` / `.TxBuffSection` sections (Ethernet-only). D2 SRAM is free.
- SD/FATFS buffers go in `SD_RAM` (`.sd_ram`) — `SDMMC` IDMA cannot reach DTCM.
- `CM4.ld`: same shape as FillStation's (AXI-SRAM 256K for M4, `RAM_D3` for BDMA,
  plus the two `INCLUDE`s).
