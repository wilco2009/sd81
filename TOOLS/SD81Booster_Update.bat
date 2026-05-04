@echo off
setlocal enabledelayedexpansion
title SD81 Booster - Actualizacion de Firmware

echo.
echo  ============================================================
echo   SD81 Booster - Actualizacion de Firmware del MCU
echo  ============================================================
echo.

:: ── 1. Buscar el archivo .bin en la carpeta del script ──────────
set "BIN_FILE="
for %%f in ("%~dp0*.bin") do (
    set "BIN_FILE=%%f"
)

if not defined BIN_FILE (
    echo  [ERROR] No se ha encontrado ningun archivo .bin en esta carpeta.
    echo.
    echo  Descarga el firmware desde el repositorio del proyecto y
    echo  copia el archivo .bin en la misma carpeta que este script.
    echo.
    goto :error
)

echo  Firmware encontrado: %BIN_FILE%
echo.

:: ── 2. Localizar STM32_Programmer_CLI ───────────────────────────
set "CLI="

:: Metodo A: PATH del sistema
where STM32_Programmer_CLI.exe >nul 2>&1
if %errorlevel%==0 (
    for /f "delims=" %%i in ('where STM32_Programmer_CLI.exe') do set "CLI=%%i"
    echo  Programador encontrado en PATH: !CLI!
    goto :found
)

:: Metodo B: Registro de Windows (64 bits)
for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\STMicroelectronics\STM32CubeProgrammer" /v InstallPath 2^>nul') do (
    set "CLI=%%b\bin\STM32_Programmer_CLI.exe"
)
if defined CLI if exist "!CLI!" (
    echo  Programador encontrado via registro: !CLI!
    goto :found
)

:: Metodo C: Registro de Windows (32 bits en sistema 64 bits)
set "CLI="
for /f "tokens=2*" %%a in ('reg query "HKLM\SOFTWARE\WOW6432Node\STMicroelectronics\STM32CubeProgrammer" /v InstallPath 2^>nul') do (
    set "CLI=%%b\bin\STM32_Programmer_CLI.exe"
)
if defined CLI if exist "!CLI!" (
    echo  Programador encontrado via registro WOW64: !CLI!
    goto :found
)

:: Metodo D: Rutas tipicas de instalacion
set "CLI="
set "TRY1=C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
set "TRY2=C:\Program Files (x86)\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"

if exist "!TRY1!" (
    set "CLI=!TRY1!"
    echo  Programador encontrado en ruta tipica: !CLI!
    goto :found
)
if exist "!TRY2!" (
    set "CLI=!TRY2!"
    echo  Programador encontrado en ruta tipica: !CLI!
    goto :found
)

:: No encontrado
echo  [ERROR] No se ha encontrado STM32CubeProgrammer en el sistema.
echo.
echo  Para actualizar el firmware necesitas instalar STM32CubeProgrammer,
echo  disponible gratuitamente en:
echo.
echo    https://www.st.com/en/development-tools/stm32cubeprog.html
echo.
echo  Una vez instalado, vuelve a ejecutar este script.
goto :error

:found
echo.

:: ── 3. Confirmar antes de flashear ──────────────────────────────
echo  Programador listo.
echo.
echo  ANTES DE CONTINUAR, asegurate de haber seguido estos pasos:
echo.
echo    1. DESCONECTA el interface del ZX81.
echo    2. Abre la carcasa del interface para acceder a la placa.
echo    3. Localiza el jumper JP7, junto al puerto USB-C.
echo    4. Puentea los dos pines SUPERIORES del JP7 (posicion de programacion).
echo    5. Conecta el cable USB-C al interface y al ordenador.
echo    6. Espera a que Windows reconozca el dispositivo.
echo.
echo  Si no has hecho estos pasos, escribe N, hazlos y vuelve a ejecutar.
echo.
set /p "CONFIRM=  Escribe S y pulsa ENTER para continuar, o N para cancelar: "
if /i "!CONFIRM!" neq "S" (
    echo.
    echo  Actualizacion cancelada. Sigue los pasos indicados y vuelve a ejecutar.
    goto :end
)

echo.
echo  Firmware a instalar:
echo    %BIN_FILE%
echo.
echo  ADVERTENCIA: No desconectes el cable durante la actualizacion.
echo.
echo  Iniciando actualizacion del firmware...
echo.

:: ── 4. Flashear ─────────────────────────────────────────────────
"%CLI%" -c port=usb1 -d "%BIN_FILE%" 0x08000000 -v -s 0x08000000

if %errorlevel%==0 (
    echo.
    echo  ============================================================
    echo   Actualizacion completada con exito.
    echo.
    echo   Ahora debes:
    echo     1. Desconectar el cable USB-C del interface.
    echo     2. Retirar el puente del JP7 (dejarlo todo abierto).
    echo     3. Cerrar la carcasa del interface.
    echo     4. Conectar el interface al ZX81 y encenderlo.
    echo  ============================================================
    echo.
    goto :end
) else (
    echo.
    echo  [ERROR] La actualizacion ha fallado.
    echo.
    echo  Comprueba que:
    echo    1. El jumper JP7 tiene puenteados los dos pines superiores.
    echo    2. El cable USB-C esta correctamente conectado.
    echo    3. El interface NO esta conectado al ZX81.
    echo    4. Los drivers DFU estan instalados correctamente.
    echo       Si es la primera vez, instala STM32CubeProgrammer y reinicia.
    echo.
    echo  Si el problema persiste, consulta el repositorio del proyecto.
    echo.
    goto :error
)

:error
echo.
pause
exit /b 1

:end
echo.
pause
exit /b 0
