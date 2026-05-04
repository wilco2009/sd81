
// #include <avr/io.h>
// #include <avr/interrupt.h>

#include <setjmp.h>
#include <EEPROM.h>
#include <SPI.h>
#include <Wire.h>
#include <SdFat.h>
#include <STM32RTC.h>
//#include <SDPlayXSVFJTAGArduino.h>
//#include <XModem.h>

#include "PINS.h"
#include "GLOBALS.h"
#include "dac_control.h"
#include "voice_synthesizer.h"
#include "play_command_parser.h"
#include "mixer.h"
#include "ay_emulator.h"
#include "WILDCARD.h"
#include "MEM.h"
#include "SD_handle.h"
//#include "VGM.h"
//#include "PEG.h"
//#include "WAV.h"
#include "COMMS.h"
#include "T81.h"
#include "transfer.h"
#include "COMMANDS.h"
#include "VBAT.h"
#include "RTC.h"

//#include "CPLD.h"

jmp_buf jbuf;

inline void jmpfar()
{
  longjmp(jbuf, 1);
}

// Try to select the best SD card configuration.
#if HAS_SDIO_CLASS
#define SD_CONFIG SdioConfig(FIFO_SDIO)
#elif ENABLE_DEDICATED_SPI
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, DEDICATED_SPI, SD_SCK_MHZ(16))
#else  // HAS_SDIO_CLASS
#define SD_CONFIG SdSpiConfig(SD_CS_PIN, SHARED_SPI, SD_SCK_MHZ(16))
#endif  // HAS_SDIO_CLASS

void setup() {
  if ((GPIOR0 & 2) != 0){
    start_reset_Z80();
  } else set_clock(!(GPIOR0 & 1));

  Serial.begin(SERIAL_SPEED);
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(ST_LED_R, OUTPUT);
  pinMode(ST_LED_G, OUTPUT);
  pinMode(ST_LED_B, OUTPUT);

  pinMode(nWAIT, INPUT);
  pinMode(CTRL_CLK, OUTPUT);
  pinMode(nOE_OL, OUTPUT);
  pinMode(LE_IL, OUTPUT);
  pinMode(Z80_RESET, OUTPUT);
//  pinMode(nMEM_LATCH_OE, OUTPUT);
  
//  pinMode(TMS, INPUT);
//  pinMode(TDI, INPUT);
//  pinMode(TDO, INPUT);
//  pinMode(TCK, INPUT);

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

//  pinMode(D0, INPUT);
//  pinMode(D1, INPUT);
//  pinMode(D2, INPUT);
//  pinMode(D3, INPUT);
//  pinMode(D4, INPUT);
//  pinMode(D5, INPUT);
//  pinMode(D6, INPUT);
//  pinMode(D7, INPUT);
//
//  pinMode(nMEM_CE, INPUT);
//  pinMode(ALE, INPUT);
//  pinMode(nMEM_OE, INPUT);
  pinMode(nWRb, INPUT);

//  pinMode(STD_48K, OUTPUT);
  pinMode(EN_M1_NOT, OUTPUT);
//  pinMode(INT_CHR_TABLE, OUTPUT);
  
  pinMode(FULLPAGING, OUTPUT);
  pinMode(SEL_128CHARS, OUTPUT);

//  pinMode(SPEAKER, OUTPUT);
  pinMode(SD_LED, OUTPUT);
  digitalWrite(FULLPAGING, LOW);
  digitalWrite(SEL_128CHARS, LOW);
  ioLatch->MODER = 0b0000000011111111; // input-output port
  digitalWrite(LE_IL, HIGH);
  digitalWrite(nOE_OL, LOW);
  digitalWrite(EN_M1_NOT, LOW);

  log_0("INIT");
  if (SD_Init()) set_status_LED(clGREEN);
  else LED_error(clBLUE);

  set_SDLed(LED_OFF);

  // Descomentar cuando este el programa de la FPGA
  // if (!(GPIOR0 & 2)){ 
  //   digitalWrite(Z80_RESET, LOW);
  //   delay(1);

  //   enableMem();
  //   int status = load_ROM("/SYS/SDBOOST.ROM");
  //   if (status == -1) LED_error(clORANGE);
  //   else if (status == -2) LED_error(clBLUE);
  //   disableMem();
  //   _rst_ctrl_reg(HIGH);
  //   _rst_data_reg(HIGH);
  // }
  // define comms functions
  attachInterrupt(digitalPinToInterrupt(GET_CTRL_REG),get_ctrl_reg,RISING);
//
  interrupts();
  log_1("Clock: %d", comm_clk);
  reseted = false;
  set_SDLed(LED_OFF);
  

  setup_dac(); 
  mixer_init(TIM1); 

  if (!(GPIOR0 & 2)){ 
    log_0("reseting Z80");
    delay(1);
    digitalWrite(Z80_RESET, HIGH);
  } 
//  playing_wav = false;
  vbat_init();
  float bat_level = get_battery_level();
  Serial.print("VBAT: "); Serial.println(bat_level);

  rtc_init();
  // Set the time
  // rtc.setHours(hours);
  // rtc.setMinutes(minutes);
  // rtc.setSeconds(seconds);

  // // Set the date
  // rtc.setWeekDay(weekDay);
  // rtc.setDay(day);
  // rtc.setMonth(month);
  // rtc.setYear(year);


  // while (1){
  // // Print date...
  // Serial.printf("%02d/%02d/%02d ", rtc.getDay(), rtc.getMonth(), rtc.getYear());

  // // ...and time
  // Serial.printf("%02d:%02d:%02d.%03d\n", rtc.getHours(), rtc.getMinutes(), rtc.getSeconds(), rtc.getSubSeconds());

  // delay(1000);
  // };
}

void setup_l() {
  if ((GPIOR0 & 2) != 0){
    start_reset_Z80();
  } else set_clock(!(GPIOR0 & 1));

  file_array = (uint16_t*)copy_buffer;

  Wire.setSDA(SDA);
  Wire.setSCL(SCL);

  Serial.begin(SERIAL_SPEED);
  log_0("INIT");
  pinMode(LED_BUILTIN, OUTPUT);

  pinMode(ST_LED_R, OUTPUT);
  pinMode(ST_LED_G, OUTPUT);
  pinMode(ST_LED_B, OUTPUT);

  pinMode(nWAIT, INPUT);
  pinMode(CTRL_CLK, OUTPUT);
  pinMode(nOE_OL, OUTPUT);
  pinMode(LE_IL, OUTPUT);
  pinMode(Z80_RESET, OUTPUT);
//  pinMode(nMEM_LATCH_OE, OUTPUT);
  
//  pinMode(TMS, INPUT);
//  pinMode(TDI, INPUT);
//  pinMode(TDO, INPUT);
//  pinMode(TCK, INPUT);

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

//  pinMode(D0, INPUT);
//  pinMode(D1, INPUT);
//  pinMode(D2, INPUT);
//  pinMode(D3, INPUT);
//  pinMode(D4, INPUT);
//  pinMode(D5, INPUT);
//  pinMode(D6, INPUT);
//  pinMode(D7, INPUT);
//
//  pinMode(nMEM_CE, INPUT);
//  pinMode(ALE, INPUT);
//  pinMode(nMEM_OE, INPUT);
  pinMode(nWRb, INPUT);

//  pinMode(STD_48K, OUTPUT);
  pinMode(EN_M1_NOT, OUTPUT);
//  pinMode(INT_CHR_TABLE, OUTPUT);
  
  pinMode(FULLPAGING, OUTPUT);
  pinMode(SEL_128CHARS, OUTPUT);

//  pinMode(SPEAKER, OUTPUT);
  pinMode(SD_LED, OUTPUT);

  // default values
//  digitalWrite(INT_CHR_TABLE, LOW);
//  digitalWrite(STD_48K, LOW);
  digitalWrite(FULLPAGING, LOW);
  digitalWrite(SEL_128CHARS, LOW);
  ioLatch->MODER = 0b0000000011111111; // input-output port
  digitalWrite(LE_IL, HIGH);
  digitalWrite(nOE_OL, LOW);
  digitalWrite(EN_M1_NOT, LOW);
  if (SD_Init()) set_status_LED(clGREEN);
  else LED_error(clBLUE);

  set_SDLed(LED_OFF);
  
  //check_CPLD_update();
  set_SDLed(LED_OFF);
  
  if (!(GPIOR0 & 2)){ 
    digitalWrite(Z80_RESET, LOW);
    delay(1);

    enableMem();
    int status = load_ROM("/SYS/SDBOOST.ROM");
    if (status == -1) LED_error(clORANGE);
    else if (status == -2) LED_error(clBLUE);
    disableMem();
    _rst_ctrl_reg(HIGH);
    _rst_data_reg(HIGH);
  }
  // define comms functions
  attachInterrupt(digitalPinToInterrupt(GET_CTRL_REG),get_ctrl_reg,RISING);
//
  interrupts();
  log_1("Clock: %d", comm_clk);
  reseted = false;
  set_SDLed(LED_OFF);
  

  // voice_init();

  // AY_init();
  // DAC_Init(DAC_AY);
  
  if (!(GPIOR0 & 2)){ 
    log_0("reseting Z80");
    delay(1);
    digitalWrite(Z80_RESET, HIGH);
  } 
//  playing_wav = false;
}

#define MAX_COMMAND_LENGTH 256 // Aumentar tamaño para múltiples cadenas
char serial_command_buffer[MAX_COMMAND_LENGTH + 1]; 
int serial_buffer_idx = 0;

// Función auxiliar para extraer una subcadena entre comillas o hasta una coma
// Retorna un puntero al inicio de la subcadena, y termina la subcadena con '\0'
// Actualiza el 'current_pos' al final de la subcadena + coma/comillas
char* extract_quoted_string(char* input_string, int* current_pos) {
    char* start_ptr = input_string + *current_pos;

    // Saltar espacios y comas iniciales
    while (*start_ptr == ' ' || *start_ptr == '\t' || *start_ptr == ',') {
        start_ptr++;
    }

    if (*start_ptr == '"') { // Si empieza con comilla
        start_ptr++; // Mover el puntero después de la comilla de apertura
        char* end_ptr = start_ptr;
        while (*end_ptr != '"' && *end_ptr != '\0') {
            end_ptr++;
        }
        if (*end_ptr == '"') {
            *end_ptr = '\0'; // Terminar la subcadena
            *current_pos = (end_ptr - input_string) + 1; // Mover pos al final de la comilla de cierre
            return start_ptr;
        }
    } else { // Si no hay comillas, leer hasta la coma o fin de cadena
        char* end_ptr = start_ptr;
        while (*end_ptr != ',' && *end_ptr != '\0') {
            end_ptr++;
        }
        char temp_char = *end_ptr; // Guardar el carácter original
        *end_ptr = '\0'; // Terminar la subcadena
        *current_pos = (end_ptr - input_string); // Mover pos al final de la subcadena
        if (temp_char == ',') { // Si se detuvo por una coma, avanza una posición más
             (*current_pos)++;
        }
        return start_ptr;
    }
    return nullptr; // No se encontró una cadena válida
}

void loop() { // Loop for test PLAY command
    // Actualiza el parser de música y el emulador AY en cada ciclo
    while (Serial.available()) {
        char inChar = Serial.read();

        if (inChar == '\n' || inChar == '\r') {
            if (serial_buffer_idx > 0) {
                serial_command_buffer[serial_buffer_idx] = '\0'; 

                if (strncmp(serial_command_buffer, "PLAY ", 5) == 0) {
                    char* command_str_raw = &serial_command_buffer[5];
                    int pos = 0;
                    
                    char* cmd_A = extract_quoted_string(command_str_raw, &pos);
                    char* cmd_B = extract_quoted_string(command_str_raw, &pos);
                    char* cmd_C = extract_quoted_string(command_str_raw, &pos);

                    // Asegurarse de que las cadenas no sean nulas si alguna falta
                    if (!cmd_A) cmd_A = "";
                    if (!cmd_B) cmd_B = "";
                    if (!cmd_C) cmd_C = "";

                    log_2("A=%s B=%s C=%s",cmd_A,cmd_B,cmd_C);

                    play_parser_start(cmd_A, cmd_B, cmd_C);

                } else if (strncmp(serial_command_buffer, "SPEECH ", 7) == 0){
                    int pos = 0;
                    char* command_str_raw = &serial_command_buffer[7];
                    char* cmd_speech = extract_quoted_string(command_str_raw, &pos);
                    if (!cmd_speech) cmd_speech = "";
                    Speech(cmd_speech); // ¡Esto iniciará la voz!
                }
                else {
                    Serial.print("Unknown command: ");
                    Serial.println(serial_command_buffer);
                }
                serial_buffer_idx = 0; 
            }
        } else {
            if (serial_buffer_idx < MAX_COMMAND_LENGTH) {
                serial_command_buffer[serial_buffer_idx++] = inChar;
            } else {
                Serial.println("Command buffer full!");
                serial_buffer_idx = 0; 
            }
        }
    }
    play_parser_update(); 
}

void loop_bak() {
  setjmp(jbuf);
  if (reseted){
    reseted = false;
    //AY_stop();
    Sfile.close();
    Dfile.close();
    dir.close();
    //VGMFile.close();
    TmpFile.close();
    if (comm_clk) GPIOR0 |= 1;
    else GPIOR0 &= B11111110;
    //noInterrupts();
    delay(100);

//    SdFile FPEG;
//    FPEG.open("/EFECT.bin");
//    FPEG.read(&PEG_mem[0],FPEG.size());
//    FPEG.close();
   
  
    //PEG_pc[0] = 0;
    //playing_PEG[0] = false;
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

//  xmodem.receive();
  idle_tasks();
  // handle serial commands
//   if (Serial.available()) {
//     char ch = Serial.read();
//     //Serial.println(ch);
//     if (ch=='R') { digitalWrite(Z80_RESET, LOW); delay(1); digitalWrite(Z80_RESET, HIGH); }
//     if (ch=='S') { 
//       ToggleClock();
//     }
// //    if (ch=='P') {  digitalWrite(ROM_WR_EN, HIGH);}
// //    if (ch=='p') {  digitalWrite(ROM_WR_EN, LOW);}
//     if (ch=='M') {  digitalWrite(EN_M1_NOT, HIGH);}
//     if (ch=='m') {  digitalWrite(EN_M1_NOT, LOW);}
//     if (ch=='G') {  showHex((unsigned char*) PEG_mem);}
//     if (ch=='W') {  playWavFile("/coche/race.WAV");}

//     if (ch=='V') { 
//       Serial.print("millis="); Serial.print(millis()); Serial.print(" tiVGM="); Serial.println(ti_VGM); 
//       if (openVGM("/music0002.vgm")==0) playing_VGM = true; }

//     if (ch=='J') { 
//       Serial.print("playing="); Serial.print(playing_VGM); Serial.print(" millis="); Serial.print(millis()); Serial.print(" tiVGM="); Serial.println(ti_VGM); }

//   }
}
