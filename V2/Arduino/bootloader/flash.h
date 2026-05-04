#ifndef FLASH_H
#define FLASH_H

#include <stdint.h>

void flash_unlock_init();
void flash_lock();
uint32_t GetSector(uint32_t address);
HAL_StatusTypeDef flash_erase_app(uint32_t firmware_size);
HAL_StatusTypeDef flash_program_firmware(File firmwareFile);
HAL_StatusTypeDef flash_verify_firmware(File firmwareFile);
void jump_to_application(uint32_t app_address);

extern char version[10];

#define BUFFER_SIZE 512
#define APPLICATION_ADDRESS 0x0800C000 //0x0800C000 
//#define APPLICATION_ADDRESS 0x08000000 
//#define APPLICATION_ADDRESS 0x08020000 
#define MAX_BOOTLOADER_SIZE (48*1024)
#define MAX_APP_SIZE (524288 - MAX_BOOTLOADER_SIZE)
#define FLASH_WRITE_UNIT sizeof(uint32_t)

#endif