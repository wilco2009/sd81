# SD81 Booster — Contexto del proyecto para Claude Code

## Descripción

El **SD81 Booster** es un interface de expansión de hardware abierto para el Sinclair ZX81. Añade carga/guardado desde microSD, hasta 512 KB de RAM, emulación del chip de sonido AY-3-8910/12, síntesis de voz, compatibilidad con Chroma81 (vídeo RGB color) y QuickSilva (128 caracteres definibles), RTC con batería, joystick DB9 programable, reproductor WAV y muchas otras funciones.

## Créditos

```
Hardware design & MCU firmware:  Alejandro Valero (wilco2009)
Z80 code / Modified ROM:         Pedro Gimeno (pgimeno)

Based on ZX81 ROM disassembly by Geoff Wearmouth
Preserved by Tomaž Šolc (https://www.tablix.org/~avian/spectrum/rom/)
```

Referencia externa: Mazogs con color Chroma81 por Pedro Gimeno — https://codeberg.org/pgimeno/Mazogs

## Componentes del proyecto

| Componente | Descripción |
|-----------|-------------|
| **MCU** | STM32F407VET (firmware en C/Arduino framework, carpeta `V2/Arduino/`) |
| **FPGA** | Xilinx Spartan-6 XC6SLX9 (lógica de pegamento, carpeta `V2/FPGA/`) |
| **ROM Z80** | ROM del ZX81 modificada (carpeta `z80rom/`) |
| **Documentación** | `README.md`, `MANUAL/`, `DOC/` |
| **Herramientas** | `TOOLS/` (scripts de actualización de firmware) |

## Estructura del repositorio

```
SD81-Booster/
├── README.md                         ← Descripción principal del proyecto
├── CLAUDE.md                         ← Este archivo
├── MANUAL/
│   ├── ES/
│   │   └── SD81_Manual_ES.md         ← Manual de usuario en español (v1.0)
│   └── EN/
│       └── SD81_Manual_EN.md         ← Manual en inglés (pendiente de traducción)
├── DOC/
│   ├── SD81 Booster TECHNICAL DOCUMENTATION.md  ← Referencia técnica (comandos MCU)
│   ├── SD81 Booster compatibility list.md        ← Lista de compatibilidad de juegos
│   ├── speak & AY synthetiser.md                 ← Documentación AY y síntesis de voz
│   └── img/                                      ← Imágenes de la documentación
├── FIRMWARE/
│   └── README_update.md              ← Instrucciones de actualización de firmware
├── TOOLS/
│   └── SD81Booster_Update.bat        ← Script Windows para actualización de emergencia vía USB
├── EXAMPLES/                         ← Programas BASIC de ejemplo
├── V2/
│   ├── Arduino/                      ← Firmware MCU (versión actual: SD81BoosterV2_038_STM32)
│   │   ├── Old/                      ← Versiones antiguas (Arduino Mega/AVR)
│   │   └── libraries/
│   ├── FPGA/                         ← Lógica FPGA Xilinx Spartan-6 XC6SLX9
│   └── Alofones/                     ← Archivos de audio de alófonos (uso interno)
└── z80rom/                           ← Código fuente de la ROM Z80 modificada
```

## Estado actual del firmware

La versión activa del firmware es **SD81BoosterV2_038_STM32** (STM32F407VET). Las carpetas `Old/` contienen versiones históricas para Arduino Mega (AVR/ATmega2560), ya obsoletas.

## Correcciones importantes aplicadas (Mayo 2025)

Durante la redacción del manual v1.0 se detectaron y corrigieron numerosas inexactitudes en la documentación existente. Ver `Updates/CORRECCIONES_REPOSITORIO.md` para el historial completo. Los puntos más críticos:

- **Chroma81**: El SD81 Booster **integra** la funcionalidad del Chroma81 (RGB) y QuickSilva (128 chars). No son incompatibles — es que no se pueden conectar dos interfaces físicamente al mismo tiempo, pero la funcionalidad está integrada.
- **Síntesis de voz**: Las muestras están en la **memoria interna del MCU** (no en `ALOFONES.DAT` ni en la SD). Basado en el chip SP0256 de General Instrument. `LOAD *SAY "texto"` acepta inglés directamente.
- **POKE 4096,1**: Ya **no es necesario** en ningún juego. Eliminar de toda la documentación.
- **Modo MC45**: La resistencia de 680Ω ya está **incorporada en el interface**. No existe jumper para deshabilitar en la versión actual.
- **Modo HRG**: Transparente, no requiere activación. Comandos 46 y 47 (ICHR) están comentados en el código — no hacen nada.
- **LED**: Se llama **STAT** (no STA). Está en el **panel superior** (no lateral).

## Comandos BASIC nuevos no documentados en versiones anteriores

| Comando | Función |
|---------|---------|
| `LOAD FAST "file.WAV"` | Reproduce audio WAV sin comprimir (PCM) |
| `LOAD *RTC` | Muestra fecha/hora del RTC |
| `LOAD *RTC TO R$` | Guarda fecha/hora en variable |
| `LOAD *RTC="cadena"` | Ajusta el reloj (6 formatos) |
| `LOAD *BAT` | Muestra voltaje batería RTC |
| `LOAD *BAT TO B$` | Guarda voltaje en variable |
| `LOAD *RAM48` | Activa modo RAM extendida 48KB |
| `LOAD *RAM48 STOP` | Desactiva modo RAM48 |
| `LOAD THEN PRINT "fichero"` | Muestra archivo de texto en pantalla |
| `LOAD THEN LPRINT "fichero"` | Envía texto a ZX Printer |
| `LOAD *JOY "OPQA "` | Configura joystick DB9 (5 chars: izq/der/arr/aba/fuego) |
| `LOAD *CD "archivo.T81"` | Navega directorio T81 (alfa) |

## Actualización de firmware

El firmware del MCU tiene dos partes diferenciadas:

| Parte | Archivo | Dirección |
|-------|---------|-----------|
| **Bootloader** | `FIRMWARE/bootloader.bin` | `0x08000000` |
| **Aplicación** | `FIRMWARE/firmware.bin` | `0x0800C000` |

**Proceso normal (bootloader vía SD):**
1. Copiar el archivo `SD81BoosterV2_038_STM32.ino.bin` renombrado como `firmware.bin` en la raíz de la SD
2. Encender el ZX81
3. El bootloader lo detecta, graba en `0x0800C000`, borra el archivo y arranca la aplicación

**Recuperación de emergencia (vía USB, para usuarios avanzados):**
1. Abrir la carcasa del interface
2. Puentear los dos pines superiores del jumper **JP7** (en la cara de componentes, junto al USB-C)
3. Conectar USB-C al PC
4. Usar **STM32CubeProgrammer** para grabar:
   - Bootloader en `0x08000000`
   - Aplicación en `0x0800C000`
5. Retirar el puente de JP7 y cerrar la carcasa

## Sistema de archivos de la SD

La carpeta **SYS** en la raíz de la SD es **imprescindible** — sin ella el interface no arranca. Contiene los archivos de ROM necesarios.

El comando `LOAD THEN PRINT "*nombre"` busca en `/MAN/nombre.TXT` — sistema de ayuda integrado al estilo `man` de Linux.

## Estados del LED STAT

**Durante actualización de firmware:**
- Azul/Rojo parpadeante → Error inicializando SD
- Amarillo fijo → Actualizando firmware
- Blanco/Rojo parpadeante → Error de actualización

**Durante arranque:**
- Rojo parpadeante → Inicializando puerto serie
- Naranja parpadeante → Esperando a la FPGA
- Azul/Rojo parpadeante → Error inicializando SD
- Naranja/Rojo parpadeante → Error escribiendo ROM en RAM
- Amarillo parpadeante → Inicializando RTC
- Verde fijo → Sistema inicializado correctamente

**Durante funcionamiento:**
- Verde fijo → Interface listo, modo QuickSilva apagado
- Cyan fijo → Interface listo, modo QuickSilva encendido

## Notas de desarrollo

- El firmware usa el framework Arduino para STM32 (STM32duino)
- Hay carpetas `configuracion_Claude` y `configuracion_Gemini` en `V2/Arduino/` con configuraciones de entorno de desarrollo generadas con IA
- Los archivos `.WAV` de alófonos en `V2/Alofones/` son para uso interno del proceso de compilación (generación de tablas), no se usan en la SD
- Formatos de archivo soportados por LOAD: `.P`, `.81`, `.P81` (multi-programa), `.ROM` (carga en dir 0 y resetea), `.WAV` (reproduce), sin extensión (carga raw)
- Directorios T81 (`.T81`): contenedores de múltiples programas, solo lectura — fase alfa
- Los índices 0–127 de GETBYTE/SETBYTE son volátiles (`sys_vars[]`); 128–255 son EEPROM persistente (offset 128)
