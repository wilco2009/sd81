# ==============================================================
# SD81 Booster - Script de restauración de configuración
# Restaura las entradas SD81 en el boards.txt del core
# STM32duino tras una actualización del core.
# ==============================================================
# Uso: click derecho -> "Ejecutar con PowerShell"
# o desde PowerShell: .\restore_SD81_boards.ps1
# ==============================================================

# --- Bloque a insertar (no modificar) ---
$sd81Block = @"

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
"@

# --- Detectar versión del core ---
$coreBase = "$env:LOCALAPPDATA\Arduino15\packages\STMicroelectronics\hardware\stm32"
$coreVersion = Get-ChildItem $coreBase | Sort-Object Name -Descending | Select-Object -First 1 -ExpandProperty Name
$boardsFile = "$coreBase\$coreVersion\boards.txt"

Write-Host ""
Write-Host "=== SD81 Booster - Restauración de configuración ===" -ForegroundColor Cyan
Write-Host "Core detectado: $coreVersion" -ForegroundColor Yellow
Write-Host "Fichero:        $boardsFile"
Write-Host ""

# --- Verificar que el fichero existe ---
if (-not (Test-Path $boardsFile)) {
    Write-Host "ERROR: No se encuentra boards.txt en $boardsFile" -ForegroundColor Red
    Read-Host "Pulsa Enter para cerrar"
    exit 1
}

# --- Comprobar si ya existen las entradas ---
$content = Get-Content $boardsFile -Raw
$appExists = $content -match "GenF4\.menu\.pnum\.SD81_BOOSTER"
$bootExists = $content -match "GenF4\.menu\.pnum\.SD81_BOOTLOADER"

if ($appExists -and $bootExists) {
    Write-Host "Ambas entradas SD81 ya existen en boards.txt." -ForegroundColor Green
    Write-Host "No es necesario restaurar."
    Write-Host ""
    Read-Host "Pulsa Enter para cerrar"
    exit 0
}

if ($appExists -or $bootExists) {
    Write-Host "AVISO: Solo una de las entradas existe. Se añadirá el bloque completo al final." -ForegroundColor Yellow
    Add-Content $boardsFile $sd81Block
} else {
    # --- Buscar la línea de anclaje (final de la entrada BLACK_F407VE) ---
    $anchorPattern = "GenF4\.menu\.pnum\.BLACK_F407VE\.debug\.svd_file"
    if ($content -notmatch $anchorPattern) {
        Write-Host "AVISO: No se encontró la línea de anclaje BLACK_F407VE.debug.svd_file" -ForegroundColor Yellow
        Write-Host "Se añadirá el bloque al final del fichero."
        Add-Content $boardsFile $sd81Block
    } else {
        $lines = Get-Content $boardsFile
        $newLines = @()
        $inserted = $false
        foreach ($line in $lines) {
            $newLines += $line
            if (-not $inserted -and $line -match $anchorPattern) {
                $newLines += $sd81Block.Split("`n")
                $inserted = $true
            }
        }
        $newLines | Set-Content $boardsFile
    }
}

# --- Verificar resultado ---
$verify = Get-Content $boardsFile -Raw
$appOk = $verify -match "GenF4\.menu\.pnum\.SD81_BOOSTER"
$bootOk = $verify -match "GenF4\.menu\.pnum\.SD81_BOOTLOADER"

if ($appOk -and $bootOk) {
    Write-Host "Ambas entradas SD81 añadidas correctamente." -ForegroundColor Green
} elseif ($appOk) {
    Write-Host "AVISO: Solo se añadió SD81_BOOSTER." -ForegroundColor Yellow
} elseif ($bootOk) {
    Write-Host "AVISO: Solo se añadió SD81_BOOTLOADER." -ForegroundColor Yellow
} else {
    Write-Host "ERROR: Las entradas no se añadieron correctamente." -ForegroundColor Red
}

Write-Host ""
Write-Host "Recuerda reiniciar Arduino IDE." -ForegroundColor Yellow
Write-Host ""
Read-Host "Pulsa Enter para cerrar"
