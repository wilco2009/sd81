// --- FUNCIONES DEL BOOTLOADER ---
#include <Arduino.h>
#include <stm32f4xx_hal_flash.h>
#include <stm32f4xx_hal_flash_ex.h>
#include <stm32f4xx_hal.h>
#include <stdint.h>
#include <SD.h>
#include "flash.h"
#include "global.h"
#include "PINS.h"

extern char version[10] = "1.0";

uint8_t read_buffer[BUFFER_SIZE*4];

// En flash.cpp:
HAL_StatusTypeDef wait_inactive_flash() {
    uint32_t timeout = 5000; 
    uint32_t start_tick = HAL_GetTick(); 
    
    while (__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY)) {
        if ((HAL_GetTick() - start_tick) > timeout) {
  #ifdef DEBUG
            Serial.println("TIMEOUT: BSY pegado.");
  #endif
            __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  #ifdef DEBUG
            Serial.print("FLASH_SR: 0x"); Serial.println(FLASH->SR, HEX);
  #endif
            return HAL_ERROR; // Devuelve un error explícito
        }
    }
    // Si la espera termina sin timeout, devuelve OK
    return HAL_OK;
}

void flash_unlock_init() {
  #ifdef DEBUG
    Serial.println("esperando flash no ocupada");
  #endif
    if (wait_inactive_flash() != HAL_OK) {
        // Si el desbloqueo falla, no tiene sentido continuar.
  #ifdef DEBUG
        Serial.println("FATAL: Flash no disponible despues de desbloqueo.");
  #endif
        // Podrías forzar un reset aquí o saltar a la aplicación principal.
    }    
  #ifdef DEBUG
    Serial.println("desbloqueando la flash");
  #endif
    HAL_FLASH_Unlock();
  #ifdef DEBUG
    Serial.println("esperando fin del desbloqueo");
  #endif
    wait_inactive_flash();
  #ifdef DEBUG
    Serial.println("fin del desbloqueo");
  #endif
    // Limpia todas las banderas de error
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
  #ifdef DEBUG
    Serial.println("flags borrados");
  #endif
}

void flash_lock() {
    HAL_FLASH_Lock();
}


uint32_t GetSector(uint32_t address) {
    if (address < 0x08004000) return FLASH_SECTOR_0;
    if (address < 0x08008000) return FLASH_SECTOR_1;
    if (address < 0x0800C000) return FLASH_SECTOR_2;
    if (address < 0x08010000) return FLASH_SECTOR_3;
    if (address < 0x08020000) return FLASH_SECTOR_4;
    if (address < 0x08040000) return FLASH_SECTOR_5; // 0x08020000 -> 0x0803FFFF
    if (address < 0x08060000) return FLASH_SECTOR_6; // 0x08040000 -> 0x0805FFFF    
    return FLASH_SECTOR_7; 
}

HAL_StatusTypeDef flash_erase_app(uint32_t firmware_size) {

    if (wait_inactive_flash() != HAL_OK) {
   #ifdef DEBUG
       Serial.println("ERROR: Borrado fallido o BSY persistente.");
        return HAL_ERROR;
  #endif
    }    
    
    FLASH_EraseInitTypeDef EraseInitStruct;

    uint32_t SectorError = 0;
    
    uint32_t startSector = GetSector(APPLICATION_ADDRESS);
    uint32_t endAddress = APPLICATION_ADDRESS + firmware_size - 1; 
    uint32_t endSector = GetSector(endAddress);
    
    if (endAddress >= (0x08000000 + MAX_APP_SIZE)) {
        return HAL_ERROR; 
    }
   
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3; 
    EraseInitStruct.Sector = startSector; 
    EraseInitStruct.NbSectors = endSector - startSector + 1; 
    
  #ifdef DEBUG
    Serial.print("Borrando sectores: ");
    Serial.print(startSector); Serial.print(" a "); Serial.println(endSector);
  #endif

    for (uint32_t sector = startSector; sector <= endSector; sector++) {
        EraseInitStruct.Sector = sector; 
        EraseInitStruct.NbSectors = 1; // ¡Borrar solo uno a la vez!
        
  #ifdef DEBUG
        Serial.print("Borrando Sector: "); Serial.println(sector);
  #endif

        if (HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError) != HAL_OK) {
            // Si HAL_FLASHEx_Erase falla y retorna HAL_ERROR, lo capturamos
  #ifdef DEBUG
            Serial.print("ERROR al borrar Sector: "); Serial.println(sector);
  #endif
            return HAL_ERROR;
        }
        
        // Aquí, inserta tu función de polling (wait_inactive_flash())
        // para esperar que el sector borrado termine (lo cual es esencial).
        if (wait_inactive_flash() != HAL_OK) {
  #ifdef DEBUG
            Serial.print("TIMEOUT de espera en Sector: "); Serial.println(sector);
  #endif
            return HAL_ERROR;
        }
    }
  #ifdef DEBUG
    Serial.println("Esperando a que el borrado termine");
  #endif
    // 3. ESPERA ACTIVA (POLLING) 
    
    // Esperar activamente a que el bit de BUSY se desactive
    return wait_inactive_flash();


    // Borrar posibles errores de estado después del borrado
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);
}


// Función robusta de escritura manual de 32 bits (WORD)
HAL_StatusTypeDef flash_program_manual_word(uint32_t Address, uint32_t Data) {
    
    // 1. Esperar a que la Flash esté lista
    if (wait_inactive_flash() != HAL_OK) return HAL_ERROR;

    // 2. Configurar el tamaño de programación a x32 (WORD)
    // PSIZE = 10 (Bits 8 y 9 del registro CR)
    // Borramos los bits de PSIZE
    FLASH->CR &= (~FLASH_CR_PSIZE); 
    // Configuramos PSIZE a '10' (Program x32)
    FLASH->CR |= FLASH_PSIZE_WORD; 

    // 3. Activar el bit de Programación (PG)
    FLASH->CR |= FLASH_CR_PG;

    // --- SECCION CRITICA: SIN INTERRUPCIONES ---
    __disable_irq(); // Desactivar interrupciones para evitar cuelgues
    
    // 4. Escribir el dato directamente (32 bits)
    *(__IO uint32_t*)Address = Data;
    
    // Barrera de sincronización de datos (Asegura que la escritura se ejecute)
    __DSB(); 
    
    __enable_irq(); // Reactivar interrupciones
    // -------------------------------------------

    // 5. Esperar a que la operación termine
    HAL_StatusTypeDef status = wait_inactive_flash();

    // 6. Desactivar el bit de Programación (PG)
    FLASH->CR &= (~FLASH_CR_PG);

    // 7. Verificar errores
    if (status != HAL_OK || (FLASH->SR & (FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR))) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

HAL_StatusTypeDef flash_program_firmware(File firmwareFile) {
    uint32_t flashAddress = APPLICATION_ADDRESS;
    size_t bytesRead;

    // Deshabilitar cachés
    __HAL_FLASH_INSTRUCTION_CACHE_DISABLE();
    __HAL_FLASH_DATA_CACHE_DISABLE();

  #ifdef DEBUG
    Serial.println("Comenzando programacion MANUAL 32-bit...");
  #endif

    while (firmwareFile.available()) {
        // Leer 512 bytes al buffer
      size_t bytesRead = firmwareFile.read(read_buffer, BUFFER_SIZE * 4);        
        // Calcular cuántas palabras de 32 bits (4 bytes) hay
        uint32_t numWords = bytesRead / 4;

        for (uint32_t i = 0; i < numWords; i++) {
            
            uint32_t dataToWrite;
            // USAR MEMCPY ES OBLIGATORIO:
            // Evita errores de alineación si read_buffer no está alineado a 4 bytes en RAM
            memcpy(&dataToWrite, &read_buffer[i * 4], 4);

            // Llamada a la nueva función de 32 bits
            if (flash_program_manual_word(flashAddress, dataToWrite) != HAL_OK) {
                uint32_t sr = FLASH->SR;
  #ifdef DEBUG
                Serial.print("Error en 0x"); Serial.print(flashAddress, HEX);
                Serial.print(" SR: 0x"); Serial.println(sr, HEX);
  #endif
                return HAL_ERROR;
            }
            
            flashAddress += 4; // Avanzar 4 bytes
        }
        
        // Feedback visual simple
  #ifdef DEBUG
        Serial.print(".");
  #endif
    }
    
  #ifdef DEBUG
    Serial.println("\nGrabacion exitosa.");
    delay(100);
  #endif

    __HAL_FLASH_INSTRUCTION_CACHE_ENABLE();
    __HAL_FLASH_DATA_CACHE_ENABLE();

    return HAL_OK;
}

HAL_StatusTypeDef flash_verify_firmware(File firmwareFile) {
#ifdef DEBUG
    Serial.println("Iniciando verificacion...");
#endif
    // 1. IMPORTANTE: Volver al inicio del archivo SD
    firmwareFile.seek(0);

    uint32_t flashAddress = APPLICATION_ADDRESS;
    uint8_t verifyBuffer[BUFFER_SIZE]; // Usamos un buffer local de bytes
    size_t bytesRead;
    uint32_t totalBytes = 0;

    while (firmwareFile.available()) {
        // Leer un bloque de la SD
        bytesRead = firmwareFile.read(verifyBuffer, BUFFER_SIZE);
#ifdef DEBUG
        Serial.print("leidos ");Serial.print(bytesRead);Serial.println(" bytes");
#endif
      
        // Puntero directo a la memoria Flash (STM32 permite leer Flash directamente)
        uint8_t* flashPtr = (uint8_t*)flashAddress;

        // Comparar memoria
        for (size_t i = 0; i < bytesRead; i++) {
            if (verifyBuffer[i] != flashPtr[i]) {
#ifdef DEBUG
                Serial.println();
                Serial.print("ERROR de verificacion en direccion: 0x");
                Serial.println(flashAddress + i, HEX);
                
                Serial.print("Esperado (SD): 0x");
                Serial.print(verifyBuffer[i], HEX);
                Serial.print(" - Leido (Flash): 0x");
                Serial.println(flashPtr[i], HEX);
#endif
                
                return HAL_ERROR; // Salir inmediatamente si falla
            }
        }

        flashAddress += bytesRead;
        totalBytes += bytesRead;
        
        // Feedback visual cada ciertos KB para no saturar la consola
#ifdef DEBUG
        if ((totalBytes % (1024*10)) == 0) Serial.print("V");
#endif
    }
#ifdef DEBUG

    Serial.println("\nVerificacion COMPLETADA con EXITO.");
#endif
    return HAL_OK;
}

void jump_to_application(uint32_t app_address) {
    void (*app_reset_handler)(void); 
    //void (*app_reset_handler)(void) = (void*)(*((volatile uint32_t*) (app_address + 4U)));
//   void (*app_reset_handler)(void) = (void*)(*((volatile uint32_t*) (0x08000000 + 4U)));
   //void (*app_reset_handler)(void) = (void (*)(void)) (app_address + 4U);
    app_reset_handler = (void (*)(void)) (*((uint32_t *)(app_address + 4)));

    HAL_RCC_DeInit();
    HAL_DeInit();
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    SCB->VTOR = app_address;

  __set_MSP(*(volatile uint32_t*) app_address);
    app_reset_handler();
    
    while(1);
}