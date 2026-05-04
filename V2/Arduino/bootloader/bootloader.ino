#include <SD.h>
#include "PINS.h"
#include <SPI.h>
#include <stm32f4xx_hal_flash.h>
#include <stm32f4xx_hal_flash_ex.h>
#include <stm32f4xx_hal.h>
#include <stdint.h>
#include "flash.h"
#include "global.h"

//#define DEBUG


char firmware_name [] = "firmware.bin";
uint32_t firmware_size;
extern "C" uint32_t SystemCoreClock;

File firmware_file;
void set_SDLed(bool state){
  digitalWrite(SD_LED, state);
}

void set_status_LED(uint8_t R,uint8_t G, uint8_t B){
#ifdef COMMON_ANODE
  analogWrite(ST_LED_R, (0xff-R)*1/1); 
  analogWrite(ST_LED_G, (0xff-G)*1/10); 
  analogWrite(ST_LED_B, (0xff-B)*1/2); 
#else
  analogWrite(ST_LED_R, R*1/1); 
  analogWrite(ST_LED_G, G*1/4); 
  analogWrite(ST_LED_B, B*1/8); 
#endif
}

void set_status_LED(uint32_t RGB){
//  set_status_LED((RGB >> 16)&0xff, (RGB >> 8)&0xff, RGB & 0xff);
  set_status_LED((RGB >> 16)&0xff, (RGB >> 8)&0xff, RGB & 0xff);
}

void LED_error(uint32_t RGB1, uint32_t RGB2){
    while (1) {
      set_status_LED(RGB1);
      delay(500);
      set_status_LED(RGB2);  
      delay(500);
     }
}
void LED_error(uint32_t RGB){
  LED_error(RGB, clRED);
}

void update_firmware(){
  firmware_size = firmware_file.size();
  set_status_LED(clRED);
  flash_unlock_init();
  flash_erase_app(firmware_size);
// PROGRAMAR POR BLOQUES
  set_status_LED(clYELLOW);
  HAL_StatusTypeDef status = flash_program_firmware(firmware_file);

  // Cerrar y Bloquear
  if (status == HAL_OK) {

    if (flash_verify_firmware(firmware_file) == HAL_OK) {
#ifdef DEBUG
        Serial.println("Firmware verificado correctamente.");
#endif
        firmware_file.close();
        SD.remove(firmware_name);

        flash_lock();
        log("written and verified");
        // Aquí ya es seguro saltar
        set_status_LED(clWHITE);
        jump_to_application(APPLICATION_ADDRESS);
    } else {
#ifdef DEBUG
        Serial.println("Fallo en la verificacion. NO SE SALTA.");
#endif
        firmware_file.close();
        flash_lock();
        log("written but verify failed");
        LED_error(clWHITE);
        while(1); 
    }
  } else {
#ifdef DEBUG
        Serial.println("Fallo en la grabacion. NO SE SALTA.");
#endif
        firmware_file.close();
        flash_lock();
        log("write failed");
        // Aquí podrías parpadear un LED de error infinito
        LED_error(clWHITE);
        while(1); 
  }

}



bool sdled = false;
void setup() {
  pinMode(SD_LED, OUTPUT);
  set_SDLed(LED_ON);
  delay(1000);
  set_SDLed(LED_OFF);
  delay(1000);
//    jump_to_application(0x08000000);
#ifdef DEBUG
  Serial.begin(115200);
  #endif
  SystemClock_Config();
  // wait for Serial Monitor to connect. Needed for native USB port boards only:
  #ifdef DEBUG
  while (!Serial);
  Serial.print("ℹ️ System core clock frequency (HCLK): ");
  Serial.print(SystemCoreClock / 1000000.0, 2); // Divide por 1,000,000 para MHz, con 2 decimales
  Serial.println(" MHz");
  Serial.print("Initializing SD card...");
  #endif
  pinMode(SS_SD, OUTPUT);
  SPI.setMOSI(MOSI);
  SPI.setMISO(MISO);
  SPI.setSCLK(SCLK);
  pinMode(ST_LED_R, OUTPUT);
  pinMode(ST_LED_G, OUTPUT);
  pinMode(ST_LED_B, OUTPUT);
  pinMode(SD_LED, OUTPUT);
  
  set_status_LED(clBLUE);

  if (!SD.begin(SS_SD)) {
  #ifdef DEBUG
    Serial.println("initialization failed. Things to check:");
    Serial.println("1. is a card inserted?");
    Serial.println("2. is your wiring correct?");
    Serial.println("3. did you change the chipSelect pin to match your shield or module?");
    Serial.println("Note: press reset button on the board and reopen this serial monitor after fixing your issue!");
  #endif
    LED_error(clBLUE);
    while (1);
 }
  #ifdef DEBUG
  Serial.println("initialization done.");
  #endif
  set_status_LED(clCYAN);

  firmware_file = SD.open(firmware_name, FILE_READ);
  if (firmware_file) {
  #ifdef DEBUG
    Serial.print(firmware_name); Serial.println(" found, updating.....");
  #endif
    update_firmware();
  } else {
    firmware_file.close();
  #ifdef DEBUG
    Serial.print(firmware_name); Serial.print(" not found, starting main program");
  #endif
    set_status_LED(clPINK);
    jump_to_application(APPLICATION_ADDRESS);
    return;  
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
