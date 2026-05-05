# SD81 Booster — Firmware Update

This document explains how to update the SD81 Booster firmware.

> If you want to recompile the firmware from source, see [COMPILING.md](COMPILING.md) instead.

---

## Firmware structure

The SD81 Booster has two programmable components, each updated independently:

### MCU (STM32F407VET)

The MCU firmware consists of two independent parts, each flashed to a separate region:

| Part | Binary | Flash address | Description |
|------|--------|--------------|-------------|
| **Bootloader** | `bootloader.bin` | `0x08000000` | Runs first on power-on. Checks the SD card for a `firmware.bin` file and flashes it automatically. Occupies sectors 0–2 (48 KB). |
| **Application** | `firmware.bin` | `0x0800C000` | The main firmware. Loaded by the bootloader after update, or directly on normal boot. |

Under normal circumstances, **only the application needs to be updated**. The bootloader rarely changes.

### FPGA (Xilinx Spartan-6 XC6SLX9)

| File | Description |
|------|-------------|
| `SD81Booster.mcs` | MCS image to be written to the auxiliary SPI flash (25Q128) connected to the FPGA. |

The FPGA loads its configuration from this SPI flash at power-up. See [the FPGA programming section](#fpga-programming-via-jtag) below for update instructions.

All binaries are included in this folder.

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

---

## FPGA programming via JTAG

This procedure is intended for **advanced users and manufacturers only**. It requires a Xilinx USB Platform Cable (or compatible clone) and Xilinx ISE iMPACT.

The FPGA (Spartan-6 XC6SLX9) does not store its configuration internally. It loads from an auxiliary SPI flash chip (25Q128) at every power-up. Programming means writing `SD81Booster.mcs` to that flash via JTAG indirect programming.

> ⚠️ **Warning:** Incorrect programming may render the interface permanently inoperative.

**Steps:**

1. Connect the JTAG cable to the JTAG header on the board (TMS — TDI — TDO — TCK — GND — VREF).
2. Power the board via USB-C.
3. Open **Xilinx ISE iMPACT** and double-click **Boundary Scan**.
4. Right-click in the Boundary Scan window → **Initialize Chain**. The FPGA (`XC6SLX9`) should appear.
5. Right-click the FPGA → **Add SPI/BPI Flash**. Browse to `SD81Booster.mcs` and select it.
6. When prompted for the flash device, select **SPI PROM → 25Q128** (128 Mbit).
7. Right-click the flash → **Program**.
8. Wait for **PROGRAM SUCCEEDED**.
9. Power-cycle the board. The FPGA will load its new configuration automatically.
