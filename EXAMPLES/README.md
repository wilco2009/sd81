# SD81 Booster — Examples

This folder contains example programs and support files demonstrating the features of the SD81 Booster.

Copy the contents to your microSD card to try them on a real ZX81.

---

## Programs (`P/`)

| File | Description |
|------|-------------|
| `AUTOEXEC.P` | Loaded and run automatically when pressing `RUN` + `NEWLINE` at startup. Can be used as a launcher or boot menu. |
| `BATMAN.P` | Batman image displayed in HiRes (high-resolution) black and white mode. |
| `DEMOSND.P` | Demonstration of the `LOAD *PLAY` command — plays a musical sequence by passing a note string to the AY synthesizer. |
| `HRSP2.P` | Demo of the HiRes Spectrum display mode. |
| `M1NOT.P` | Test program for the MC45 mode (machine code execution in the 8000h–BFFFh range). |
| `MENU3.P` | Example navigation menu. |
| `SAY.P` | Demonstration of the speech synthesizer (`LOAD *SAY`). |
| `SPCHR-64.P` | Loads and displays the Spectrum character set in standard 64-character mode. |
| `SPCHR-128.P` | Loads and displays the Spectrum character set in 128-character mode. |
| `C64CHR-64.p` | Loads and displays the Commodore 64 character set in standard 64-character mode. |
| `C64CHR-128.p` | Loads and displays the Commodore 64 character set in 128-character mode. |
| `clkdebug.p` | Developer tool for debugging RTC synchronization when implementing new commands. Not intended for end users. |
| `hrtland3.p` | Homebrew game by a third party, included as a HiRes demo. |

### Support files

| File | Description |
|------|-------------|
| `spec-81.bin` | Spectrum character set bitmap — used by `SPCHR-64.P`. |
| `spec-81-128.bin` | Spectrum character set bitmap (128 chars) — used by `SPCHR-128.P`. |
| `C64-81.bin` | Commodore 64 character set bitmap — used by `C64CHR-64.p`. |
| `C64-81-128.bin` | Commodore 64 character set bitmap (128 chars) — used by `C64CHR-128.p`. |
| `DEMO.wav` | Uncompressed WAV audio file — example for `LOAD FAST "DEMO.WAV"`. |
| `STOP.wav` | Short WAV audio file — example for WAV playback. |
| `z.scr` | Spectrum screen (.SCR format) — used as a display demo in HiRes Spectrum mode. |
| `VGM/music0002.vgm` | AY music file in VGM format — example for `LOAD *VGM`. |

---

## PEG effects (`PEG/`)

The PEG (Programmable Effect Generator) folder contains example sound effects and the assembler used to create them.

| File | Description |
|------|-------------|
| `efect.peg` | PEG source code for a sound effect (human-readable assembly). |
| `efect.peb` | Compiled PEG binary — ready to load with `LOAD *PEG`. |
| `test2.peg` | Second PEG effect source. |
| `test2.peb` | Compiled binary for `test2.peg`. |
| `peg.py` | Python script that assembles `.peg` source files into `.peb` binaries. Requires Python 3. |

To assemble a PEG source file:

```bash
python peg.py efect.peg efect.peb
```

---

## VGM music (`VGM/`)

| File | Description |
|------|-------------|
| `music0002.vgm` | AY music file in VGM format. Play it with `LOAD *VGM "VGM/music0002.vgm"`. |

> Note: the SD81 Booster VGM player only supports AY-3-8910/12 opcodes. VGM files containing other chips will not play correctly.
