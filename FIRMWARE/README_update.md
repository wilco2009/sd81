# SD81 Booster — Firmware Update

This document explains how to update the SD81 Booster firmware.

> If you want to recompile the firmware from source, see [COMPILING.md](COMPILING.md) instead.

---

## Firmware structure

The MCU firmware consists of two independent parts, each flashed to a separate region:

| Part | Binary | Flash address | Description |
|------|--------|--------------|-------------|
| **Bootloader** | `bootloader.bin` | `0x08000000` | Runs first on power-on. Checks the SD card for a `firmware.bin` file and flashes it automatically. Occupies sectors 0–2 (48 KB). |
| **Application** | `firmware.bin` | `0x0800C000` | The main firmware. Loaded by the bootloader after update, or directly on normal boot. |

Both binaries are included in this folder.

Under normal circumstances, **only the application needs to be updated**. The bootloader rarely changes.

---

## Normal update (via SD card)

This is the recommended method for all users.

1. Copy `firmware.bin` from this folder to the root of the microSD card.
2. Insert the SD card into the interface.
3. Power on the ZX81.
4. The bootloader detects `firmware.bin`, flashes it to `0x0800C000`, deletes the file from the SD card, and boots the new application automatically.

The STAT LED shows the following during the process:

| LED state | Meaning |
|-----------|---------|
| Yellow fixed | Flashing in progress — **do not power off** |
| Green fixed | Update successful, application running |
| Blue/Red blinking | Error reading SD card |
| White/Red blinking | Flash write error |

### Verifying the update

Once the ZX81 has booted, run the following command to check the firmware version:

```
LOAD *VER
```

---

## Emergency recovery (via USB)

Use this method only if the normal SD update fails or the application is corrupted and the interface does not boot correctly. Requires opening the case.

**Requirements:**
- [STM32CubeProgrammer](https://www.st.com/en/development-tools/stm32cubeprog.html) (free download from ST)
- A USB-C cable

**Steps:**

1. **Disconnect** the interface from the ZX81.
2. Open the interface case to access the PCB.
3. Locate jumper **JP7** on the component side of the board, next to the USB-C port.
4. Bridge the **two upper pins** of JP7 to put the MCU in programming mode.
5. Connect the USB-C cable to the interface and to the PC.
6. Wait for the PC to recognise the device (it should appear as a DFU device in STM32CubeProgrammer).
7. Flash the two binaries in order:

| Step | Binary | Address |
|------|--------|---------|
| 1 | `bootloader.bin` | `0x08000000` |
| 2 | `firmware.bin` | `0x0800C000` |

8. Once flashing is complete, **remove the JP7 bridge**.
9. Close the case and reconnect the interface to the ZX81.

> **Note:** Only flash the bootloader if it is known to be corrupted. In most cases, flashing only the application (`0x0800C000`) is sufficient.
