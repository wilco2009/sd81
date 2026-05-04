# Script para concatenar el contenido binario de archivos en una carpeta a un archivo destino
# Ahora acepta un patrón de archivo con comodines (ej. "*.bin", "parte_*.rom", "doc??.txt")
# CORRECCIÓN: Usa FileStream para la escritura binaria en modo de añadir.

# --- Configuración (Modifica estas rutas y el patrón según tus necesidades) ---

# Ruta de la carpeta que contiene los archivos a sumar
$sourceFolder = "C:\Users\aleja\OneDrive\Documentos\git\github\SD81-Booster - Privado\V2\SD Content\SYS\VOICE" # Asegúrate que esta ruta es correcta y existe

# Ruta y nombre del archivo destino donde se guardará el resultado
# Puedes usar Join-Path para que el archivo de salida esté en la misma carpeta que el script
$destinationFile = Join-Path $PSScriptRoot "output_sumado.bin"
# O, si prefieres una ruta específica (¡asegúrate que la carpeta exista!):
# $destinationFile = "C:\Users\aleja\Desktop\output_sumado.bin"

# Patrón de archivo con comodines.
$filePattern = "E4*.wav" # Asegúrate de que esta es la extensión correcta para tus archivos de origen

# --- Lógica del Script ---

Write-Host "Iniciando la suma de archivos binarios..."
Write-Host "Carpeta de origen: $sourceFolder"
Write-Host "Archivo destino: $destinationFile"
Write-Host "Patrón de archivo: $filePattern"
Write-Host "--------------------------------------------------"

# Verificar si la carpeta de origen existe
if (-not (Test-Path $sourceFolder -PathType Container)) {
    Write-Error "¡Error! La carpeta de origen '$sourceFolder' no existe."
    exit 1
}

# Obtener todos los archivos de la carpeta de origen que coincidan con el patrón,
# ordenados alfabéticamente para una concatenación consistente.
$filesToConcatenate = Get-ChildItem -Path $sourceFolder -Filter $filePattern | Sort-Object Name

# Verificar si se encontraron archivos
if ($filesToConcatenate.Count -eq 0) {
    Write-Warning "No se encontraron archivos que coincidan con el patrón '$filePattern' en '$sourceFolder'."
    exit 0
}

# Eliminar el archivo de destino si ya existe para asegurar un inicio limpio
if (Test-Path $destinationFile) {
    Remove-Item $destinationFile -Force
    Write-Host "  Archivo destino existente eliminado: '$destinationFile'."
}

# Abrir el FileStream para escritura en modo de añadir.
# [System.IO.FileMode]::Append asegura que los datos se añaden al final del archivo.
# Si el archivo no existe, FileMode.Append lo crea.
try {
    $outputStream = New-Object System.IO.FileStream($destinationFile, [System.IO.FileMode]::Append, [System.IO.FileAccess]::Write)

    foreach ($file in $filesToConcatenate) {
        Write-Host "  Concatenando: $($file.Name)..."
        try {
            # Lee el contenido del archivo como bytes
            $fileBytes = [System.IO.File]::ReadAllBytes($file.FullName)
            # Escribe los bytes en el FileStream de salida
            $outputStream.Write($fileBytes, 0, $fileBytes.Length)
        }
        catch {
            Write-Error "Error al procesar el archivo '$($file.Name)': $($_.Exception.Message)"
        }
    }
}
catch {
    Write-Error "Error al abrir o escribir en el archivo destino '$destinationFile': $($_.Exception.Message)"
}
finally {
    # MUY IMPORTANTE: Asegurarse de cerrar y liberar el FileStream
    if ($outputStream) {
        $outputStream.Close()
        $outputStream.Dispose()
    }
}

Write-Host "--------------------------------------------------"
Write-Host "¡Proceso completado!"
Write-Host "Todos los archivos que coinciden con '$filePattern' han sido concatenados en '$destinationFile'."