# SD81 Booster — Plan de reorganización del repositorio

**Fecha de elaboración:** Mayo 2025  
**Propósito:** Guía completa para reorganizar el repositorio de Codeberg con la ayuda de Claude Code, corrigiendo la documentación obsoleta, añadiendo la que falta y estructurando el proyecto de cara al lanzamiento público.

---

## 1. Estructura propuesta del repositorio

```
SD81-Booster/
│
├── README.md                        ← Reescribir (ver sección 3)
│
├── MANUAL/
│   ├── ES/
│   │   ├── SD81_Manual_ES.md        ← Manual de usuario en español (NUEVO - ya redactado)
│   │   └── img/                     ← Imágenes del manual
│   └── EN/
│       └── SD81_Manual_EN.md        ← Manual en inglés (PENDIENTE - traducir del ES)
│
├── DOC/
│   ├── SD81_Technical_Reference.md  ← Documentación técnica (REESCRIBIR - ver sección 4)
│   ├── SD81_Command_Reference.md    ← Referencia de comandos MCU (NUEVO - ya redactado en apéndice manual)
│   ├── SD81_Compatibility_List.md   ← Lista de compatibilidad de juegos (CORREGIR - ver sección 5)
│   ├── speak_and_AY.md             ← AY y síntesis de voz (REESCRIBIR - ver sección 6)
│   └── img/
│
├── FIRMWARE/
│   ├── README_update.md             ← Instrucciones de actualización de firmware
│   └── [binarios .bin cuando estén disponibles]
│
├── TOOLS/
│   └── SD81Booster_Update.bat       ← Script de actualización Windows (NUEVO - ya generado)
│
├── EXAMPLES/
│   ├── README.md                    ← Descripción de los ejemplos
│   ├── SPEC-81-128/                 ← Ejemplo juego de caracteres Spectrum
│   ├── RTC_demo/                    ← Ejemplo RTC
│   └── SUPERFAST_demo/              ← Ejemplo modo Superfast
│
└── SYS/                             ← Carpeta del sistema para la SD (estructura actual)
    └── [ROMs y archivos de sistema]
```

---

## 2. Archivos generados y listos para incluir

Los siguientes archivos han sido redactados durante el proceso de documentación y están listos para incluirse en el repositorio:

| Archivo | Estado | Destino en el repo |
|---------|--------|--------------------|
| `SD81_Manual_ES.md` | ✅ Listo | `MANUAL/ES/` |
| `SD81Booster_Update.bat` | ✅ Listo | `TOOLS/` |
| `APENDICE_A_notas.md` | ✅ Listo (notas de trabajo) | Integrado en manual |
| `APENDICE_C_diccionario_voz.md` | ✅ Listo | Integrado en manual |

**Pendiente de generar:**
- Manual en inglés (traducción del español)
- `README_update.md` para la carpeta FIRMWARE
- `README.md` de la carpeta EXAMPLES

---

## 3. README.md — Correcciones y reescritura

El README actual contiene información obsoleta e incorrecta. Debe reescribirse completamente con los siguientes cambios:

### Correcciones obligatorias

| # | Problema actual | Corrección |
|---|----------------|------------|
| 1 | "Devices that replace the internal ROM, such as Chroma81, will not work" | El SD81 Booster **integra** la funcionalidad del Chroma81 (RGB) y del QuickSilva (128 chars). No son interfaces compatibles para conectar simultáneamente, sino funcionalidades integradas. |
| 2 | Lista de características incompleta | Añadir: RTC con pila CR2032, joystick DB9 programable, reproductor WAV, modos Superfast y Spectrum, T81 (alfa), consola de depuración USB |
| 3 | Sección MC45 — menciona jumper para deshabilitar y resistencia externa | El interface ya incluye la resistencia internamente. No existe jumper para deshabilitar en la versión actual. |
| 4 | Sección "What it doesn't do" — incompleta | Actualizar con las nuevas limitaciones reales |
| 5 | Sección "Possible compatibility issues" — incompleta | Actualizar |
| 6 | Créditos ausentes | Añadir: Alejandro Valero (wilco2009) — hardware y firmware MCU; Pedro Gimeno (pgimeno) — código Z80 / ROM |

### Contenido a añadir en el README

- Descripción del sistema de actualización via bootloader (SD)
- Referencia al Mazogs de Pedro Gimeno: https://codeberg.org/pgimeno/Mazogs
- Instrucciones básicas de inicio (o enlace al manual)
- Tabla de LED STAT con los estados correctos (ver sección siguiente)

---

## 4. DOC/SD81 Booster TECHNICAL DOCUMENTATION — Correcciones

### Sección: Command Description

| Cmd | Problema | Corrección |
|-----|----------|------------|
| 9 (LOAD) | Solo documenta `.P` | Añadir: `.81`, `.P81`, `.ROM` (resetea), `.WAV` (reproduce), sin extensión (carga raw) |
| 11 (TYPE) | No documentado | Nuevo comando. `LOAD THEN PRINT`. Prefijo `*` busca en `/MAN/` + `.TXT`. Prefijo `?` = texto nativo ZX81 |
| 22 (BINARY_SAY) | Documenta hex con bancos E4/E8/E9 | Acepta array de bytes de alófonos en binario, no hex |
| 23 (SAY) | Sistema de bancos hex obsoleto | Acepta texto ASCII directamente. Con `*` al inicio: background. Basado en SP0256. |
| 29 (FULLPAGING) | No documentado | Nuevo. Activa paginación completa 512KB/64 páginas |
| 30 (HALFPAGING) | No documentado | Nuevo. Activa paginación simple 256KB/32 páginas |
| 32 (GETBYTE) | "256 bytes de memoria interna" | Índices 0–127: variables volátiles. Índices 128–255: EEPROM (persistente) |
| 33 (SETBYTE) | Idem | Idem |
| 38 (LOOP_VGM) | Sin detalle | 1 byte: 0=no bucle, otro valor=bucle |
| 21 (JOY) | No documentado | 5 bytes en códigos ZX81 nativos (no ASCII): izq/der/arr/aba/fuego |
| 46 (ENABLE_ICHR) | Documentado como activo | Función comentada en código. No hace nada. Marcar como obsoleto. |
| 47 (DISABLE_ICHR) | Documentado como activo | Idem |
| 48 (ENABLE_48K) | No documentado | Nuevo. Activa modo RAM extendida 48KB |
| 49 (DISABLE_48K) | No documentado | Nuevo. Desactiva modo RAM extendida |
| 50 (RTC) | No documentado | Nuevo. Sin params: devuelve fecha/hora. Con params: ajusta. 6 formatos de entrada. |
| 52 (BAT) | No documentado | Nuevo. Devuelve voltaje batería RTC: 5 bytes ASCII formato `V.mmm` |

### Sección: Tabla de estados del LED

Reemplazar completamente por la tabla correcta organizada en tres fases:

**Actualización de firmware:**
- Parpadeo Azul/Rojo → Error inicializando SD
- Amarillo fijo → Actualizando firmware
- Parpadeo Blanco/Rojo → Error de actualización

**Arranque:**
- Rojo parpadeante → Inicializando puerto serie
- Naranja parpadeante → Esperando a la FPGA
- Parpadeo Azul/Rojo → Error inicializando SD
- Parpadeo Naranja/Rojo → Error escribiendo ROM en RAM
- Parpadeo Amarillo → Inicializando RTC
- Verde fijo → Sistema inicializado correctamente

**Funcionamiento:**
- Verde fijo → Interface listo, modo Quick Silva apagado
- Cyan fijo → Interface listo, modo Quick Silva encendido

### Sección: Actualización de firmware

Reemplazar el proceso de actualización via USB/DFU por el proceso actual via bootloader:
1. Copiar `firmware.bin` en la raíz de la SD
2. Encender el ZX81
3. El bootloader detecta el archivo, graba en 0x0800C000, borra el archivo y salta a la aplicación

Para recuperación de emergencia via USB: ver instrucciones detalladas en el repositorio (requiere JP7 — jumper accesible abriendo la carcasa, en la cara de componentes junto al puerto USB-C, puentear los dos pines superiores para modo programación).

---

## 5. DOC/SD81 Booster compatibility list — Correcciones

| Juego | Cambio |
|-------|--------|
| BOOSTER (SOFTFARM) | Eliminar `POKE 4096,1` |
| FORTY NINER | Eliminar `POKE 4096,1` |
| ROCKET MAN | Eliminar `POKE 4096,1` |
| Z-XTRICATOR | Eliminar `POKE 4096,1` |
| Against the Element (Paul Farrow) | Eliminar `POKE 4096,1` |
| Nanako in Classic Japanese Monster Castle 81 | Eliminar `POKE 4096,1` |
| SUPER REFRIED GUN OPERATION '81 | Eliminar `POKE 4096,1` |
| FSCAPES | Verificar si sigue sin funcionar o ha mejorado con versión actual |
| Dungeon of Ymir | Idem |

---

## 6. DOC/speak & AY synthetiser — Reescritura completa

Este documento debe reescribirse completamente. El sistema actual es radicalmente diferente al documentado.

### Lo que hay que eliminar
- Todo el sistema de `ALOFONES.DAT`
- La carpeta `/SYS/VOICE/`
- El script `alofones.py`
- Las instrucciones para añadir sonidos con Audacity
- La tabla de sonidos con códigos hexadecimales de banco (E4/$80, E8/$80, etc.)
- Las instrucciones de los comandos 22 y 23 con formato hex

### Lo que hay que añadir
- Descripción del sintetizador basado en SP0256 (General Instrument)
- Referencias: Currah MicroSpeech (ZX Spectrum) y The Voice (Videopac G7000/Odyssey 2)
- Comando `LOAD *SAY "texto"` — acepta texto en inglés directamente
- Reproducción background con `*` al inicio de la cadena
- Tabla completa del diccionario de palabras reconocidas (ver Apéndice C del manual)
- Lista de alófonos directos del SP0256 (PA1-PA5, vocales, consonantes)
- Tabla de pausas por puntuación (espacio=corta, coma=media, punto y coma=larga)
- Lectura automática de números (0 hasta billones) en inglés

---

## 7. Documentación nueva a crear

### 7.1 Comandos BASIC — referencia rápida

Crear un documento de referencia rápida con todos los comandos del BASIC extendido del SD81 Booster en formato tabla, sin explicaciones largas. Ya está redactado como parte del manual (secciones 6-12 + apéndices). Extraerlo como documento independiente.

### 7.2 Guía de actualización de firmware

Crear `FIRMWARE/README_update.md` con:
- Proceso normal via bootloader (SD)
- Proceso de recuperación via USB (JP7 + STM32CubeProgrammer)
- Script `SD81Booster_Update.bat` para Windows
- Verificación de versión con `LOAD *VER`

### 7.3 Guía de inicio rápido

Extraer la sección "Guía de inicio rápido" del manual como documento independiente, ideal para incluir en la caja del producto o como primer documento del repositorio.

### 7.4 Ejemplos BASIC

Crear la carpeta `EXAMPLES/` con los programas de ejemplo documentados en el manual (sección 13):
- `RTC_demo.p` — demostración del reloj en tiempo real
- `BAT_demo.p` — lectura de batería
- `MC45_test.p` — comprobación del modo MC45
- `SUPERFAST_demo.p` — demostración de velocidad
- `SCR_viewer.p` — visor de imágenes .SCR en modo Spectrum

### 7.5 Carpeta MAN (sistema de ayuda integrado)

El comando `LOAD THEN PRINT "*nombre"` busca en `/MAN/nombre.TXT`. Crear archivos de ayuda básicos:
- `/MAN/PLAY.TXT` — ayuda del comando PLAY
- `/MAN/SAY.TXT` — ayuda del comando SAY
- `/MAN/RTC.TXT` — ayuda del comando RTC
- `/MAN/JOY.TXT` — ayuda del comando JOY
- `/MAN/VGM.TXT` — ayuda del comando VGM

---

## 8. Funcionalidades en fase alfa (marcar claramente)

Las siguientes funcionalidades deben documentarse con aviso explícito de fase alfa:

- **Directorios T81** — contenedores de múltiples programas ZX81. Las operaciones de escritura no están soportadas.

---

## 9. Créditos a incluir en toda la documentación

```
Hardware design & MCU firmware: Alejandro Valero (wilco2009)
Z80 code / Modified ROM:        Pedro Gimeno (pgimeno)

Based on ZX81 ROM disassembly by Geoff Wearmouth
Preserved by Tomaž Šolc (https://www.tablix.org/~avian/spectrum/rom/)
```

---

## 10. Checklist de tareas para Claude Code

- [ ] Crear estructura de carpetas propuesta
- [ ] Mover/renombrar archivos de documentación actuales
- [ ] Reescribir README.md
- [ ] Reescribir DOC/SD81 Booster TECHNICAL DOCUMENTATION
- [ ] Reescribir DOC/speak & AY synthetiser.md
- [ ] Corregir DOC/SD81 Booster compatibility list.md (eliminar POKE 4096,1)
- [ ] Copiar SD81_Manual_ES.md a MANUAL/ES/
- [ ] Copiar SD81Booster_Update.bat a TOOLS/
- [ ] Crear FIRMWARE/README_update.md
- [ ] Crear EXAMPLES/ con los programas de ejemplo
- [ ] Crear carpeta MAN/ con archivos de ayuda básicos
- [ ] Añadir créditos en todos los documentos principales
- [ ] Verificar todos los enlaces internos tras la reorganización
