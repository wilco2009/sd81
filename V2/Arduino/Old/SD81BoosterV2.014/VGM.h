uint32_t ti_VGM = 0;
boolean playing_VGM = false;
uint8_t VGM_mode = 0;
SdFile VGMFile;


uint8_t openVGM(char* s){
  char id [5];
  int16_t result;

  ti_VGM = 0;
  log_3("OPEN VGM %s", s);
  if (!VGMFile.open(s)) 
    return 1;
  log_3("succesfull,0");
    
  result = VGMFile.read(id, 4);
  id[4] = 0;
  if ((result != 4) || (strcmp(id, "Vgm ")!=0)){
    VGMFile.close();    
    log_3("id %s not recognized", id);
    return 11;
  }
  VGMFile.seekSet(256); // skip header  
  log_3("skiping header",0);
  DAC_Init(DAC_AY);
  
  return 0;
}

void read_next_VGM_command(){
  int16_t result;
  uint16_t samples;
  uint8_t reg;
  uint8_t val;

  ti_VGM = millis();
  result = VGMFile.read();

  log_3("VGM command %02X", result);

  if ((result >= 0x66) && (result <= 0x95)){  // 0 byte
    
  }

  if (((result >= 0x30) && (result <= 0x3F)) || (result == 0x4f) || (result == 0x50)){ // 1 byte
    
  }

  if (((result >= 0x40) && (result <= 0x4E)) || ((result >= 0x51) && (result <= 0x5F)) || ((result >= 0B0) && (result <= 0xBF)) || ((result >= 0xA1) && (result <= 0xAF))){ // 2 bytes
    
  }

  if (((result >= 0xC0) && (result <= 0xDF))){ // 3 bytes
    
  }

  if ((result >= 0xE0) && (result <= 0xEF)){ // 4 bytes
    
  }

  uint32_t ti;
  switch (result) {
    
    case 0x61:              // Wait n samples, n can range from 0 to 65535 (approx 1.49 seconds). Longer pauses than this are represented by multiple wait commands. (44100samp/s)
      samples = VGMFile.read()+256*VGMFile.read();
      ti = (uint32_t) samples*1000/44100;
      ti_VGM = millis()+ti;
      log_2("WAIT %lu ms samples=%u", ti,samples);
     
      break;
    case 0x62:             // wait 735 samples (60th of a second), a shortcut for 0x61 0xdf 0x02
      ti_VGM = millis()+17;  
      log_2("WAIT 17 ms",0);
      break;
    case 0x63:             // wait 882 samples (50th of a second), a shortcut for 0x61 0x72 0x03
      log_2("WAIT 20 ms",0);
      break;
    case 0xA0:             // AY8910, write value dd to register aa
      result = VGMFile.read();
      if (result >= 0) {
        reg = result;
        result = VGMFile.read();
        if (result >= 0){ 
          val = result;
          log_2("REG %hu = %hu", reg, val);
          AY_SetReg(reg, val);
        } else 
          log_3("Error reading value",0);
      } else
        log_3("Error reading register",0);
      if (result < 0) {
        VGMFile.close();
        playing_VGM = false; 
      }
      break;
    case -1:
      if (VGM_mode == 1) VGMFile.seekSet(256); // repeat again  
      else {
        log_3("Error reading command",0);
        VGMFile.close();
        playing_VGM = false;
      } 
      break;
  }
}
