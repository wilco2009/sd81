#include <avr/io.h>
#include <avr/interrupt.h>

#include <setjmp.h>
#include <EEPROM.h>
#include <SPI.h>
#include <SdFat.h>
#include <SDPlayXSVFJTAGArduino.h>

#include "PINS.h"
#include "GLOBALS.h"
#include "DAC.h"
#include "VOICE_TABLE.h"
#include "VOICE.h"
#include "AYEM.h"
#include "WILDCARD.h"
#include "MEM.h"
#include "SD_handle.h"
#include "COMMS.h"
#include "VGM.h"
#include "COMMANDS.h"
#include "CPLD.h"

jmp_buf jbuf;

void jmpfar()
{
  longjmp(jbuf, 1);
}

void setup() {
  if ((GPIOR0 & 2) != 0){
    start_reset_Z80();
  } else set_clock(!(GPIOR0 & 1));

  file_array = (uint16_t*)copy_buffer;


  Serial.begin(115200);
  log_0("INIT",0);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(ST_LED_R, OUTPUT);
  pinMode(ST_LED_G, OUTPUT);
  pinMode(ST_LED_B, OUTPUT);

  pinMode(nWAIT, INPUT);
  pinMode(CTRL_CLK, OUTPUT);
  pinMode(nOE_OL, OUTPUT);
  pinMode(LE_IL, OUTPUT);
  pinMode(Z80_RESET, OUTPUT);
  pinMode(nMEM_LATCH_OE, OUTPUT);
  
  pinMode(TMS, INPUT);
  pinMode(TDI, INPUT);
  pinMode(TDO, INPUT);
  pinMode(TCK, INPUT);

  pinMode(RST_CTRL_REG, OUTPUT);
  pinMode(RST_DATA_REG, OUTPUT);
  pinMode(GET_CTRL_REG, INPUT);
  pinMode(GET_DATA_REG, INPUT);

  pinMode(A18b, INPUT);
  pinMode(A17b, INPUT);
  pinMode(A16b, INPUT);
  pinMode(A15b, INPUT);
  pinMode(A14b, INPUT);
  pinMode(A13b, INPUT);
  pinMode(A12b, INPUT);
  pinMode(A11b, INPUT);
  pinMode(A10b, INPUT);
  pinMode(A9b, INPUT);
  pinMode(A8b, INPUT);

  pinMode(D0, INPUT);
  pinMode(D1, INPUT);
  pinMode(D2, INPUT);
  pinMode(D3, INPUT);
  pinMode(D4, INPUT);
  pinMode(D5, INPUT);
  pinMode(D6, INPUT);
  pinMode(D7, INPUT);

  pinMode(nMEM_CE, INPUT);
  pinMode(ALE, INPUT);
  pinMode(nMEM_OE, INPUT);
  pinMode(nWRb, INPUT);

  pinMode(SPARE1, INPUT);
  pinMode(EN_M1_NOT, OUTPUT);
  pinMode(ROM_WR_EN, OUTPUT);
  
  pinMode(FULLPAGING, OUTPUT);
  pinMode(SEL_128CHARS, OUTPUT);

  pinMode(SPEAKER, OUTPUT);
  pinMode(SD_LED, OUTPUT);

  // default values
  digitalWrite(ROM_WR_EN, LOW);
  digitalWrite(FULLPAGING, LOW);
  digitalWrite(SEL_128CHARS, LOW);
  DDRF = 0b00000000; // input port
  DDRK = 0b11111111; // output port
  digitalWrite(LE_IL, HIGH);
  digitalWrite(nOE_OL, LOW);
  digitalWrite(EN_M1_NOT, LOW);
  SD_Init(); 
  
  check_CPLD_update();
  set_SDLed(LED_OFF);
  
  if (!(GPIOR0 & 2)){ 
  
    enableMem();
    load_ROM("/SYS/SDBOOST.ROM");
    disableMem();
    _rst_ctrl_reg(HIGH);
    _rst_data_reg(HIGH);
  }
  // define comms functions
  attachInterrupt(digitalPinToInterrupt(GET_CTRL_REG),get_ctrl_reg,RISING);
//
  interrupts();
  log_1("Clock: %d", clock);
  reseted = false;
  set_SDLed(LED_OFF);
  

  voice_init();

  AY_init();
  DAC_Init(DAC_AY);
  
  if (!(GPIOR0 & 2)){ 
    log_0("reseting Z80",0);
    delay(1);
    digitalWrite(Z80_RESET, HIGH);
  } 

}

void loop() {
  setjmp(jbuf);
  if (reseted){
    reseted = false;
    AY_stop();
    Sfile.close();
    Dfile.close();
    dir.close();
    TmpFile.close();
    if (clock) GPIOR0 |= 1;
    else GPIOR0 &= B11111110;
    //noInterrupts();
    delay(100);
    //__asm("jmp 0");
  }
  set_SDLed(LED_OFF);
  data_ready = false;
  read_data();
  // call to active command handler
  if (command_active<=LAST_COMMAND) {
    log_1("COMMAND=%d",command_active);
    (*commands[command_active])();
    log_1("AFTER COMMAND=%d",command_active);
  } else {
    if (command_active != CMD_IDLE){
      log_1("Unknown command: %d",command_active);
      command_active = CMD_IDLE;
    }
  }

  if (playing_VGM) {
    log_3("millis %lu, TIVGM %lu", millis(), ti_VGM);
    if (millis() >= ti_VGM) {
      read_next_VGM_command();
    } // else waiting.......
    
  }

  // handle serial commands
  if (Serial.available()) {
    char ch = Serial.read();
    //Serial.println(ch);
    if (ch=='R') { digitalWrite(Z80_RESET, LOW); delay(1); digitalWrite(Z80_RESET, HIGH); }
    if (ch=='S') { 
      ToggleClock();
    }
    if (ch=='P') {  digitalWrite(ROM_WR_EN, HIGH);}
    if (ch=='p') {  digitalWrite(ROM_WR_EN, LOW);}
    if (ch=='M') {  digitalWrite(EN_M1_NOT, HIGH);}
    if (ch=='m') {  digitalWrite(EN_M1_NOT, LOW);}

    if (ch=='V') { if (openVGM("/music0002.vgm")==0) playing_VGM = true; }


  }
}
