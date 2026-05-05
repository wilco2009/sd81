# SD81 Booster — Compiling from Source

This document explains how to set up the development environment to recompile the SD81 Booster firmware from source.

> **Note:** This is only needed if you want to modify the firmware. For regular firmware updates, see [README_update.md](README_update.md).

---

## Overview

The firmware consists of two independent projects, each compiled to a different flash region:

| Project | Flash address | Max size | Board target |
|---------|--------------|----------|--------------|
| Bootloader | `0x08000000` | 48 KB (sectors 0–2) | `SD81Booster Bootloader` |
| Application | `0x0800C000` | ~480 KB | `SD81Booster Aplicación` |

Both are compiled with the **Arduino framework for STM32** (STM32duino), using custom board definitions derived from the `STM32 Black F407VE`.

Source code:
- Bootloader: `V2/Arduino/bootloader/`
- Application: `V2/Arduino/SD81BoosterV2_038_STM32/`

---

## Prerequisites

### 1. Arduino IDE

Download and install **Arduino IDE 2.x** from [https://www.arduino.cc/en/software](https://www.arduino.cc/en/software).

### 2. STM32duino core

In Arduino IDE, go to **File → Preferences** and add the following URL to *Additional boards manager URLs*:

```
https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json
```

Then go to **Tools → Board → Boards Manager**, search for `STM32` and install **STM32 by STMicroelectronics**.

### 3. Libraries

Install the following libraries via **Tools → Manage Libraries**:

- **SdFat** 2.2.0 by Bill Greiman
- **STM32duino RTC** by STMicroelectronics

---

## Installing the Custom Board Definitions

The SD81 Booster requires two custom board targets that are not included in the standard STM32duino core. They must be added to the `boards.txt` file of the installed core.

The board definitions to add are the following — **do not modify these entries**:

```
##############################################################
# SD81Booster Aplicación (sector >= 3 - 0x0800C000)
GenF4.menu.pnum.SD81_BOOSTER=SD81Booster Aplicación (sector >= 3 - 0x0800C000)
GenF4.menu.pnum.SD81_BOOSTER.build.board=BLACK_F407VE
GenF4.menu.pnum.SD81_BOOSTER.build.product_line=STM32F407xx
GenF4.menu.pnum.SD81_BOOSTER.build.variant=STM32F4xx/F407V(E-G)T_F417V(E-G)T
GenF4.menu.pnum.SD81_BOOSTER.build.variant_h=variant_BLACK_F407VX.h
GenF4.menu.pnum.SD81_BOOSTER.build.peripheral_pins=-DCUSTOM_PERIPHERAL_PINS
GenF4.menu.pnum.SD81_BOOSTER.upload.maximum_size=491520
GenF4.menu.pnum.SD81_BOOSTER.upload.maximum_data_size=131072
GenF4.menu.pnum.SD81_BOOSTER.build.flash_offset=0xC000
GenF4.menu.pnum.SD81_BOOSTER.build.extra_flags=-DSTM32F407xx -DARDUINO_BLACK_F407VE -Wl,--defsym=LD_FLASH_OFFSET=0xC000
GenF4.menu.pnum.SD81_BOOSTER.upload.protocol=dfu
GenF4.menu.pnum.SD81_BOOSTER.upload.options=-g 0x0800C000
GenF4.menu.pnum.SD81_BOOSTER.upload.tool=stm32CubeProg-dfu
GenF4.menu.pnum.SD81_BOOSTER.debug.svd_file={runtime.tools.STM32_SVD.path}/svd/STM32F4xx/STM32F407.svd

# SD81Booster Bootloader (solo sectores 0-2, 48KB)
GenF4.menu.pnum.SD81_BOOTLOADER=SD81Booster Bootloader (solo sectores 0-2, 48KB)
GenF4.menu.pnum.SD81_BOOTLOADER.build.board=BLACK_F407VE
GenF4.menu.pnum.SD81_BOOTLOADER.build.product_line=STM32F407xx
GenF4.menu.pnum.SD81_BOOTLOADER.build.variant=STM32F4xx/F407V(E-G)T_F417V(E-G)T
GenF4.menu.pnum.SD81_BOOTLOADER.build.variant_h=variant_BLACK_F407VX.h
GenF4.menu.pnum.SD81_BOOTLOADER.build.peripheral_pins=-DCUSTOM_PERIPHERAL_PINS
GenF4.menu.pnum.SD81_BOOTLOADER.upload.maximum_size=49152
GenF4.menu.pnum.SD81_BOOTLOADER.upload.maximum_data_size=131072
GenF4.menu.pnum.SD81_BOOTLOADER.build.flash_offset=0x0
GenF4.menu.pnum.SD81_BOOTLOADER.build.extra_flags=-DSTM32F407xx -DARDUINO_BLACK_F407VE
GenF4.menu.pnum.SD81_BOOTLOADER.upload.protocol=dfu
GenF4.menu.pnum.SD81_BOOTLOADER.upload.options=-g 0x08000000
GenF4.menu.pnum.SD81_BOOTLOADER.upload.tool=stm32CubeProg-dfu
GenF4.menu.pnum.SD81_BOOTLOADER.debug.svd_file={runtime.tools.STM32_SVD.path}/svd/STM32F4xx/STM32F407.svd
##############################################################
```

The entries should be inserted after the `GenF4.menu.pnum.BLACK_F407VE.debug.svd_file` line in `boards.txt`.

### Windows — automatic (recommended)

A PowerShell script is included in `V2/Arduino/configuracion boards.txt/` that locates the correct `boards.txt` automatically, checks whether the entries already exist, and inserts them in the right place.

From a PowerShell window:

```powershell
PowerShell -ExecutionPolicy Bypass -File ".\restore_SD81_boards.ps1"
```

Or right-click the script and select **Run with PowerShell**.

The script detects the installed core version automatically and inserts the block after the `BLACK_F407VE` anchor. If the anchor is not found, the block is appended at the end. It also checks whether the entries already exist to avoid duplicates.

After running the script, **restart Arduino IDE**.

### Windows — manual

The `boards.txt` file is located at:

```
%LOCALAPPDATA%\Arduino15\packages\STMicroelectronics\hardware\stm32\<version>\boards.txt
```

Open it with a text editor, find the line:

```
GenF4.menu.pnum.BLACK_F407VE.debug.svd_file=...
```

and insert the block above immediately after it. Save the file and restart Arduino IDE.

### macOS — manual

The `boards.txt` file is located at:

```
~/Library/Arduino15/packages/STMicroelectronics/hardware/stm32/<version>/boards.txt
```

Open it with any text editor (e.g. TextEdit in plain text mode, or `nano` / `code` from Terminal), find the `BLACK_F407VE.debug.svd_file` line, insert the block after it, save and restart Arduino IDE.

From Terminal:

```bash
BOARDS=$(ls ~/Library/Arduino15/packages/STMicroelectronics/hardware/stm32/ | sort -V | tail -1)
open ~/Library/Arduino15/packages/STMicroelectronics/hardware/stm32/$BOARDS/boards.txt
```

### Linux — manual

The `boards.txt` file is located at:

```
~/.arduino15/packages/STMicroelectronics/hardware/stm32/<version>/boards.txt
```

From Terminal:

```bash
BOARDS=$(ls ~/.arduino15/packages/STMicroelectronics/hardware/stm32/ | sort -V | tail -1)
nano ~/.arduino15/packages/STMicroelectronics/hardware/stm32/$BOARDS/boards.txt
```

Find the `BLACK_F407VE.debug.svd_file` line, insert the block after it, save (`Ctrl+O`, `Ctrl+X`) and restart Arduino IDE.

> **Note after core updates:** Updating the STM32duino core overwrites `boards.txt` and removes the custom entries. Re-run the PowerShell script (Windows) or repeat the manual insertion (macOS/Linux) after each core update.

---

## Selecting the Board in Arduino IDE

After adding the custom definitions and restarting Arduino IDE:

1. Go to **Tools → Board → STM32 boards groups → Generic STM32F4 series**.
2. Go to **Tools → Board part number** and select:
   - `SD81Booster Bootloader (solo sectores 0-2, 48KB)` — to compile the bootloader
   - `SD81Booster Aplicación (sector >= 3 - 0x0800C000)` — to compile the application

---

## Compiling

Open the corresponding `.ino` file in Arduino IDE and click **Sketch → Verify/Compile** (or press `Ctrl+R`).

The compiled `.bin` file will be placed in the `build/STMicroelectronics.stm32.GenF4/` subfolder of the sketch directory.

---

## Flashing after compilation

Use **STM32CubeProgrammer** to flash the compiled binaries:

| Binary | Flash address |
|--------|--------------|
| `bootloader.bin` | `0x08000000` |
| `firmware.bin` | `0x0800C000` |

Remember to put the interface in programming mode first (bridge JP7 upper pins, connect USB-C). See [README_update.md](README_update.md) for the full procedure.
