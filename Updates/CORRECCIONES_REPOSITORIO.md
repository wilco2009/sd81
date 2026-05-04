# Correcciones pendientes en el repositorio SD81 Booster

**Fecha:** Mayo 2025  
**Origen:** Detectadas durante la redacción del manual de usuario v1.0  
**Referencia cruzada:** Ver `REPO_REORGANIZACION.md` para el plan de acción completo.

---

## MANUAL.md

### 1. Compatibilidad con Chroma81
- **Estado actual en repo:** "Devices that replace the internal ROM, such as Chroma81, will not work with this interface."
- **Corrección:** El SD81 Booster **integra** la funcionalidad del Chroma81 (RGB color) y del QuickSilva (128 chars). Eliminar la mención como ejemplo de incompatibilidad.

### 2. Síntesis de voz — muestras de audio
- **Estado actual en repo:** Muestras en `ALOFONES.DAT` en `/SYS/VOICE/`. Script `alofones.py`. Sonidos personalizables con Audacity.
- **Corrección:** Muestras en memoria interna del MCU. No existe `ALOFONES.DAT` ni la carpeta `VOICE`. No es posible añadir sonidos personalizados.

### 3. POKE 4096,1
- **Estado actual en repo:** Indicado como necesario para: BOOSTER, FORTY NINER, ROCKET MAN, Z-XTRICATOR, Against the Element, Nanako, SUPER REFRIED GUN OPERATION '81.
- **Corrección:** Ya no es necesario. Eliminar de toda la documentación.

### 4. Modo MC45 — jumper y resistencia
- **Estado actual en repo:** Usuario debe insertar resistencia de 680Ω en pin 18 del Z80, o retirar jumper del interface.
- **Corrección:** Resistencia ya incorporada en el interface. No existe jumper para deshabilitar en la versión actual. El riesgo de daño es muy bajo (forzado intermitente y breve). Advertencia a suavizar.

### 5. Modo HRG (LOAD *ICHR)
- **Estado actual en repo:** Comando necesario para activar modo HRG.
- **Corrección:** Modo HRG transparente, no requiere activación. Comandos 46 y 47 están comentados en el código (no hacen nada). Marcar como obsoletos o eliminar.

---

## DOC/speak & AY synthetiser.md

### 6. Arquitectura de síntesis de voz — reescritura completa
- **Eliminar:** Sistema `ALOFONES.DAT`, script `alofones.py`, carpeta `/SYS/VOICE/`, personalización con Audacity, tabla de sonidos con códigos hex de banco.
- **Añadir:** Sistema basado en SP0256 (General Instrument). Referencia a Currah MicroSpeech y The Voice. `LOAD *SAY "texto"` acepta inglés directamente. Background con `*`. Diccionario completo (ver Apéndice C del manual).

### 7. Comandos SAY 22 y 23 — formato obsoleto
- **Corrección:** Cmd 22: acepta bytes de alófonos en binario (no hex). Cmd 23: acepta texto ASCII. Sistema de bancos E4/E8/E9 eliminado.

### 8. Referencia al SP0256
- **Añadir:** Sintetizador basado en SP0256 de General Instrument, igual que el Currah MicroSpeech (ZX Spectrum) y The Voice (Videopac G7000/Odyssey 2).

---

## DOC/SD81 Booster compatibility list.md

### 9. POKE 4096,1
- **Corrección:** Eliminar de: BOOSTER, FORTY NINER, ROCKET MAN, Z-XTRICATOR, Against the Element, Nanako, SUPER REFRIED GUN OPERATION '81.

---

## README.md

### 10. Mención a Chroma81 como incompatible
- **Corrección:** El SD81 Booster integra la funcionalidad equivalente. No son interfaces para conectar simultáneamente sino funcionalidades integradas.

---

## DOC/SD81 Booster TECHNICAL DOCUMENTATION

### 11. Comando 9 — LOAD (formatos adicionales)
- **Añadir:** `.81` = igual que `.P`. `.P81` = multi-programa. `.ROM` = carga en dir 0, resetea sistema (no retorna al BASIC). `.WAV` = reproduce audio. Sin extensión = carga raw.

### 12. Comando 11 — TYPE (no documentado)
- **Añadir:** Nuevo. Implementa `LOAD THEN PRINT`. Prefijo `*` busca en `/MAN/` + `.TXT`. Prefijo `?` = texto nativo ZX81.

### 13. Comandos 22 y 23 — SAY (obsoleto)
- Ver punto 7 arriba.

### 14. Comandos 29 y 30 — FULLPAGING / HALFPAGING (no documentados)
- **Añadir:** Cmd 29 = paginación completa 512KB/64 páginas. Cmd 30 = paginación simple 256KB/32 páginas.

### 15. Comandos 32 y 33 — GETBYTE / SETBYTE (corrección)
- **Corrección:** Índices 0–127 = variables volátiles (`sys_vars[]`). Índices 128–255 = EEPROM persistente (offset 128).

### 16. Comandos 46 y 47 — ICHR (obsoletos)
- **Corrección:** Cuerpo de ambas funciones completamente comentado en el código. No hacen nada. Marcar como obsoletos.

### 17. Comando 38 — LOOP_VGM (sin detalle)
- **Añadir:** 1 byte: 0 = no bucle, cualquier otro valor = bucle activo.

### 18. Comando 21 — JOY (no documentado)
- **Añadir:** 5 bytes en códigos ZX81 nativos (no ASCII): izquierda/derecha/arriba/abajo/fuego.

### 19. Comandos 48 y 49 — ENABLE_48K / DISABLE_48K (no documentados)
- **Añadir:** Cmd 48 = activa RAM extendida 48KB. Cmd 49 = desactiva.

### 20. Comandos 50 y 52 — RTC y BAT (no documentados)
- **Añadir:** Cmd 50 (RTC): sin params devuelve fecha/hora; con params ajusta el reloj (6 formatos). Cmd 52 (BAT): devuelve voltaje batería como string `V.mmm`.

### 21. Tabla de estados del LED STAT — reescritura completa
- **El LED se llama STAT, no STA.**
- **Los LEDs están en el panel superior, no en el lateral.**
- **Eliminar:** todos los estados relacionados con grabación de CPLD, estado "blanco parpadeante".
- **Tabla correcta:** ver sección 2.4 del manual de usuario (`SD81_Manual_ES.md`).

### 22. Proceso de actualización de firmware — reescritura
- **Eliminar:** proceso via DFU/USB con `busybox.exe` y `stm32CubeProg.sh`.
- **Sustituir por:** bootloader via SD — copiar `firmware.bin` en raíz de SD, encender ZX81. Bootloader graba en `0x0800C000`, borra el archivo y salta a la aplicación.
- **Recuperación de emergencia:** JP7 (en cara de componentes junto al USB-C, puentear dos pines superiores) + STM32CubeProgrammer + `SD81Booster_Update.bat`.

---

## Funcionalidades nuevas no documentadas en el repositorio

### 23. WAV — reproducción de audio
- `LOAD FAST "fichero.WAV"` reproduce WAV sin comprimir (PCM). Detección automática por extensión.

### 24. RTC — reloj en tiempo real
- Pila CR2032 preinstalada. `LOAD *RTC`, `LOAD *RTC TO R$`, `LOAD *RTC="cadena"`. 6 formatos de fecha/hora.

### 25. BAT — estado de batería del RTC
- `LOAD *BAT`, `LOAD *BAT TO B$`.

### 26. RAM48 — modo RAM extendida
- `LOAD *RAM48`, `LOAD *RAM48 STOP`.

### 27. THEN PRINT — visualización de texto
- `LOAD THEN PRINT "fichero"`, `LOAD THEN LPRINT "fichero"`. Con `*nombre` busca en `/MAN/nombre.TXT`.

### 28. JOY — joystick programable
- `LOAD *JOY "OPQA "` — 5 caracteres: izq/der/arr/aba/fuego.

### 29. T81 — directorios contenedor *(fase alfa)*
- `LOAD *CD "archivo.T81"` navega el contenido como directorio. Solo lectura.

### 30. Consola de depuración USB
- Puerto USB-C con chip CH340G. 115200-8N1. Firmware de producción emite mensajes básicos. Con macro `DEBUG` activa en recompilación: salida detallada.

### 31. Modos Superfast y Spectrum
- POKEs en 2043-2055. Tres variantes (texto, HiRes nativo, HiRes Spectrum). Puerto FBh en modo Spectrum (incompatible con ZX Printer).

### 32. Puerto Chroma81 (7FEFh)
- Bit 0 en lectura = estado VSync. Útil para sincronización equivalente al HALT del Spectrum.

### 33. Botón QSILVA
- Alterna entre juego de caracteres QuickSilva y ROM estándar con cada pulsación.

### 34. Arranque con ROM alternativa
- Archivos `/SYS/0.ROM` a `/SYS/9.ROM`. Mantener tecla correspondiente durante el arranque.

### 35. Carpeta SYS — imprescindible
- Sin la carpeta SYS el interface **no arranca**. Contiene los archivos de ROM. Debe quedar claro en toda la documentación de instalación.

---

## Créditos a añadir en toda la documentación

```
Hardware design & MCU firmware:  Alejandro Valero (wilco2009)
Z80 code / Modified ROM:         Pedro Gimeno (pgimeno)

Based on ZX81 ROM disassembly by Geoff Wearmouth
Preserved by Tomaž Šolc (https://www.tablix.org/~avian/spectrum/rom/)
```

Software de ejemplo recomendado:
- Mazogs con color Chroma81 por Pedro Gimeno: https://codeberg.org/pgimeno/Mazogs


---

## MANUAL.md

### 1. Compatibilidad con Chroma81
- **Estado actual en repo:** Se menciona el Chroma81 como ejemplo de interface incompatible ("Devices that replace the internal ROM, such as Chroma81, will not work with this interface").
- **Corrección:** El SD81 Booster incorpora la funcionalidad del interface Chroma81 (salida de vídeo RGB color). Eliminar esa mención o sustituirla por una aclaración de compatibilidad funcional.

### 2. Síntesis de voz — muestras de audio
- **Estado actual en repo:** La documentación indica que las muestras de audio para los alófonos están almacenadas en un fichero `ALOFONES.DAT` en la carpeta `/SYS/VOICE/` de la SD, y que pueden añadirse sonidos personalizados mediante el script `alofones.py`.
- **Corrección:** Las muestras de audio están ahora en la memoria interna del microcontrolador. No existe el fichero `ALOFONES.DAT` ni la carpeta `VOICE`. No es posible añadir sonidos personalizados. Eliminar toda la sección de personalización de sonidos.

### 3. POKE 4096,1
- **Estado actual en repo:** La lista de compatibilidad indica `POKE 4096,1` como paso previo necesario para varios juegos (BOOSTER, FORTY NINER, ROCKET MAN, Z-XTRICATOR, Against the Element, Nanako, SUPER REFRIED GUN OPERATION '81).
- **Corrección:** Este POKE ya no es necesario. Eliminar de la lista de compatibilidad y de cualquier referencia en la documentación.

### 4. Modo MC45 — jumper y resistencia
- **Estado actual en repo:** Se indica que el usuario debe insertar una resistencia de 680 ohmios entre el pin 18 del Z80 y el PCB del ZX81 si el Z80 es CMOS, o retirar un jumper del interface para deshabilitar la función.
- **Corrección:**
  - El interface ya incorpora internamente la resistencia de protección. No es necesaria ninguna modificación en el ZX81.
  - No existe jumper para deshabilitar el modo en la versión actual del hardware.
  - El riesgo de daño es muy bajo ya que el forzado del pin M1 es intermitente y de muy corta duración. Suavizar el aviso en consecuencia.

### 5. Modo HRG (LOAD *ICHR)
- **Estado actual en repo:** Se documenta el comando `LOAD *ICHR` / `LOAD *ICHR STOP` como necesario para activar el modo de alta resolución en algunos juegos.
- **Corrección:** El modo de alta resolución es ahora compatible de forma transparente sin necesidad de este comando. Revisar si el comando sigue existiendo (para compatibilidad) o si puede eliminarse de la documentación completamente.

---

## DOC/speak & AY synthetiser.md

### 6. Sistema de síntesis de voz — arquitectura completa
- **Estado actual en repo:** Documenta el sistema basado en `ALOFONES.DAT`, el script `alofones.py`, la carpeta `/SYS/VOICE/`, y el proceso para añadir sonidos personalizados con Audacity.
- **Corrección:** Todo este sistema ha sido reemplazado. Las muestras están en la memoria interna del microcontrolador. No se pueden añadir sonidos personalizados. Reescribir esta sección completamente.

### 7. Comandos SAY (22 y 23) — formato hexadecimal
- **Estado actual en repo:** El comando SAY se documenta con códigos hexadecimales de banco (E4, E8, E9...) y sonido. La tabla de sonidos lista palabras y frases completas asociadas a códigos hex.
- **Corrección:** El comando `LOAD *SAY` acepta ahora texto en inglés directamente y lo convierte a fonemas internamente usando un diccionario basado en el SP0256. El sistema de bancos y códigos hexadecimales ya no es la interfaz de usuario. Actualizar completamente.

### 8. Referencia al SP0256
- **Corrección a añadir:** Documentar que el sintetizador de voz se basa en los fonemas del chip SP0256 de General Instrument, el mismo usado por el Currah MicroSpeech (ZX Spectrum) y The Voice (Videopac G7000/Odyssey 2).

---

## DOC/SD81 Booster compatibility list.md

### 9. POKE 4096,1 en lista de compatibilidad
- **Corrección:** Eliminar la columna/nota de `POKE 4096,1` de todos los juegos donde aparece (ver punto 3 arriba).

---

## README.md

### 10. Mención a Chroma81 como incompatible
- **Estado actual en repo:** "Devices that replace the internal ROM, such as Chroma81, will not work with this interface."
- **Corrección:** Aclarar que el SD81 Booster incorpora la funcionalidad equivalente al Chroma81 (vídeo RGB color) y al QuickSilva (128 caracteres definibles). No es que sea compatible con esos interfaces conectados físicamente al mismo tiempo, sino que integra sus funcionalidades.

---

## Notas generales

- La carpeta `SYS` en la SD es **imprescindible** para el arranque: contiene los archivos de ROM necesarios. Debe quedar claro en toda la documentación que sin ella el interface no arranca.
- El comando `LOAD *SAY` utiliza reproducción **síncrona** por defecto (la CPU espera). Con `*` al inicio de la cadena la reproducción es en **background**. Este comportamiento es igual al comando `LOAD *PLAY`.
- El modo background con `*` al inicio de la cadena aplica tanto a `LOAD *PLAY` (canal A) como a `LOAD *SAY`.

---

## Funciones no documentadas en el repositorio (a añadir)

### 11. Reproducción de archivos WAV
- `LOAD FAST "fichero.WAV"` reproduce un archivo WAV sin comprimir (PCM) desde la SD. El interface detecta la extensión automáticamente y reproduce el audio en lugar de cargar el archivo como programa.

### 12. Reloj en tiempo real — Comando RTC
- `LOAD *RTC` — muestra la fecha y hora actuales en pantalla.
- `LOAD *RTC TO R$` — guarda la fecha y hora en una variable de cadena.
- `LOAD *RTC="cadena"` — pone el reloj en hora. Formatos admitidos:
  - `AAAA-MM-DD HH:MM:SS.CC` — fecha y hora completas con centésimas
  - `AAAA-MM-DD HH:MM:SS` — fecha y hora completas
  - `AAAA-MM-DD` — solo fecha
  - `HH:MM:SS.CC` — solo hora con centésimas
  - `HH:MM:SS` — solo hora
  - `HH:MM` — hora y minutos (segundos a cero)
- El interface incorpora un RTC con batería de reserva.

### 13. Estado de la batería del RTC — Comando BAT
- `LOAD *BAT` — muestra el estado de la batería del RTC en pantalla.
- `LOAD *BAT TO B$` — guarda el estado en una variable de cadena.

### 14. Modo RAM extendida — Comando RAM48
- `LOAD *RAM48` — activa el modo de RAM extendida de 48 KB.
- `LOAD *RAM48 STOP` — desactiva el modo para restaurar compatibilidad estándar.

### 15. Visualización de archivos de texto — Comando THEN PRINT
- `LOAD THEN PRINT "fichero"` — muestra el contenido de un archivo de texto en pantalla (equivalente a TYPE/cat).
- `LOAD THEN LPRINT "fichero"` — envía el contenido a la impresora ZX Printer.
- `LOAD THEN PRINT "*nombre"` — busca `/MAN/nombre.TXT` en la SD y lo muestra. Sistema de ayuda integrado al estilo del comando `man` de Linux.

### 16. Joystick programable — Comando JOY
- `LOAD *JOY "cadena"` — configura el mapeo de botones del joystick DB9 a teclas del ZX81.
- La cadena tiene exactamente 5 caracteres en orden: izquierda / derecha / arriba / abajo / fuego.
- Ejemplo: `LOAD *JOY "OPQA "` — O=izq, P=der, Q=arr, A=aba, espacio=fuego.

### 17. Tabla de estados del LED STA
- Eliminar de la documentación todos los estados relacionados con la grabación de la CPLD. Esa funcionalidad nunca ha salido a la luz.
- Eliminar el estado "blanco parpadeante" (actualización de firmware).
- El LED se llama **STAT**, no STA. Corregir en toda la documentación.
- Estados válidos: verde fijo, amarillo fijo, azul fijo, rojo fijo, púrpura parpadeante, rojo parpadeante rápido.
- Los LEDs **no están en el panel lateral**, están en el **panel superior**.

---

## Correcciones detectadas en SD81 Booster TECHNICAL DOCUMENTATION (sección Command Description)

### 18. Comando 22 — ya no es BINARY SAY con hex
- **Estado actual en repo:** El comando 22 se documenta como "binary say" con una cadena hexadecimal en codificación ZX81.
- **Corrección según código:** El comando 22 (`cmd_binary_say`) acepta un array de bytes de alófonos directamente en binario (no hex). El comando 23 (`cmd_say`) acepta texto en ASCII y lo convierte internamente a fonemas usando el sintetizador basado en SP0256. El sistema de bancos hex (E4, E8, E9...) ya no es la interfaz de usuario en ninguno de los dos comandos.

### 19. Comando 11 — TYPE (nuevo, no documentado)
- El comando 11 (`cmd_type`) implementa la funcionalidad `LOAD THEN PRINT`. No estaba en la documentación técnica original.
- Soporta un prefijo `*` en el nombre para buscar en `/MAN/` con extensión `.TXT`.
- Soporta un prefijo `?` para tratar el archivo como texto nativo ZX81 (sin conversión de codificación).

### 20. Comandos 16, 17, 18 — OPENDIR, GETROWLEN, GETROW (revisión)
- El comando 16 (`cmd_opendir2`) construye un array interno de hasta 512 entradas con los índices de directorio reales (no los nombres). El array se accede después con los comandos 17 y 18.
- El comando 17 (`cmd_getrowlen`) recibe el **índice en el array** (2 bytes, little-endian), no el número de fila directamente.
- El comando 18 (`cmd_getrow`) con índice 0 devuelve el **directorio actual**, no una entrada del array. Los directorios se devuelven entre `<` y `>`.
- La documentación original describía el funcionamiento de forma simplificada; el mecanismo real usa un array de índices de directorio internos de SdFat.

### 21. Comandos 29 y 30 — FULLPAGING / HALFPAGING (nuevos, no documentados)
- Comando 29 (`cmd_fullpaging`): activa el modo de paginación completa (512 KB, 64 páginas). Equivale a `LOAD *MAP` en modo extendido.
- Comando 30 (`cmd_halfpaging`): activa el modo de paginación simple (256 KB, 32 páginas).
- No estaban documentados en el repositorio.

### 22. Comandos 32 y 33 — GETBYTE / SETBYTE (corrección)
- **Estado actual en repo:** Documentados como acceso a una memoria interna de 256 bytes del MCU.
- **Corrección según código:** Los índices 0–127 acceden a variables de sistema **volátiles** (`sys_vars[]`). Los índices 128–255 acceden a la **EEPROM** (persistente entre reinicios), con offset de 128 (índice 128 = EEPROM posición 0).

### 23. Comandos 46 y 47 — ICHR (eliminados funcionalmente)
- **Estado actual en repo:** Los comandos 46 (`cmd_enable_ich_table`) y 47 (`cmd_disable_ich_table`) gestionan el generador interno de caracteres HRG.
- **Corrección según código:** El cuerpo de ambas funciones está **completamente comentado** en el código fuente. Estos comandos no hacen nada en la versión actual. El modo HRG es transparente y no requiere activación explícita. Eliminar de la documentación técnica o marcar como obsoletos.

### 24. Comando 38 — LOOP_VGM (corrección de protocolo)
- **Estado actual en repo:** No documentado con detalle.
- **Corrección según código:** Recibe 1 byte: `VGM_mode`. El valor exacto no está definido como constante en el código visible, pero 0 = no bucle, cualquier otro valor = bucle activo.

### 25. Comando 21 — JOY (detalle de codificación)
- Los 5 bytes enviados son **códigos ZX81 nativos** (no ASCII), tal como se lee en el código: `params[i] = GetByteFromZ80_IT()` sin conversión `asc81_to_ascii`. Esto es diferente del resto de comandos que reciben strings en codificación ZX81 y los convierten a ASCII internamente.

### 26. Comando 9 — LOAD (soporte de formatos adicionales)
- **Estado actual en repo:** Solo se documentan archivos `.P`.
- **Corrección según código:** El comando LOAD detecta la extensión y actúa diferente según el formato:
  - `.P` y `.81`: calcula el tamaño real del programa desde las variables del sistema ZX81 (bytes 11-12 del fichero).
  - `.P81`: formato multi-programa, salta el nombre del fichero antes de leer las variables del sistema.
  - `.ROM`: carga en dirección 0, resetea Z80 y FPGA, no devuelve control al BASIC.
  - `.WAV`: reproduce el audio en lugar de cargar datos.
  - Sin extensión reconocida: carga el fichero completo tal cual.

### 27. Soporte de directorios T81 *(fase alfa)*
- **No documentado en el repositorio:** El interface soporta un formato de directorio especial llamado **T81** (archivos con extensión `.T81`). Cuando el directorio actual es un archivo T81, las operaciones de fichero (LOAD, DIR, OPENDIR) operan sobre el contenido del archivo T81 en lugar del sistema de archivos FAT normal. Las operaciones de escritura (DEL, MKDIR, RMDIR, MOVE, COPY, SAVE) devuelven error 13 ("operación no permitida") en directorios T81.
- **Acción:** Añadir documentación con aviso de fase alfa. Documentar la navegación mediante `LOAD *CD "archivo.T81"` y `LOAD *CD "/"` para salir. Indicar las limitaciones de escritura.

### 28. Botón QSILVA — descripción correcta
- **Estado actual en repo:** No documentado el botón QSILVA.
- **Corrección:** El botón QSILVA **alterna** entre el juego de caracteres del interface QuickSilva y el juego de caracteres estándar de la ROM del ZX81. No activa/desactiva una "compatibilidad", sino que conmuta entre dos juegos de caracteres con cada pulsación.

