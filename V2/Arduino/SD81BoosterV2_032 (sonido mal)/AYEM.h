#define SYSTEM_FREQ 1773450
//#define SYSTEM_FREQ 2000000  
#define MIN_PULSE_HIGH 32

#define AY_CH_A 0
#define AY_CH_B 1
#define AY_CH_C 2

#define R_FINE_TUNE_A     000
#define R_COARSE_TUNE_A   001
#define R_FINE_TUNE_B     002
#define R_COARSE_TUNE_B   003
#define R_FINE_TUNE_C     004
#define R_COARSE_TUNE_C   005
#define R_NOISE_PERIOD    006
#define R_ENABLE          007
#define R_AMPLITUDE_A     010
#define R_AMPLITUDE_B     011
#define R_AMPLITUDE_C     012
#define R_FINE_EP         013
#define R_COARSE_EP       014
#define R_ENV_SHAPE       015


uint8_t AY_reg[016]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0};

uint8_t RegVol[] = {R_AMPLITUDE_A, R_AMPLITUDE_B, R_AMPLITUDE_C};
uint8_t RegFTune[] = {R_FINE_TUNE_A, R_FINE_TUNE_B, R_FINE_TUNE_C};
uint8_t RegCTune[] = {R_COARSE_TUNE_A, R_COARSE_TUNE_B, R_COARSE_TUNE_C};
uint8_t whiteKeys[] = {9,11,0,2,4,5,7};
uint8_t rfine[] = {R_FINE_TUNE_A, R_FINE_TUNE_B, R_FINE_TUNE_C};
uint8_t rcoarse[] = {R_COARSE_TUNE_A, R_COARSE_TUNE_B, R_COARSE_TUNE_C};

//  f=reloj/(v*16)
 

#define PULSE_HIGH 255
#define PULSE_LOW 0

void AY_set_env(uint8_t env_typ, uint16_t env_period);

//uint8_t levels[16] = {0,31,47,63,79,95,111,127,143,159,175,191,207,223,239,255};
uint8_t levels[16] = {0,1,2,3,5,7,10,15,22,31,44,63,90,127,180,255};
//uint16_t OCTAVE8[12] = {4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902};

// duracion en negras multiplicado por 8
uint16_t note_len[12] = {30,45,60,90,120,180,240,360,480,20,40,80};

#define AYRegToTone(fine, coarse){ \
  SYSTEM_FREQ / ((uint32_t)coarse*256+fine)/16  \ 
}


uint16_t AYRegToEnv(uint8_t fine, uint8_t coarse){
  uint16_t result;

////  uint64_t EP = (uint16_t)(coarse*256)+fine;
//  uint16_t c = coarse;
//  uint16_t f = fine;
//  uint64_t EP = (c*256)+f;
//  uint64_t sfreq = SYSTEM_FREQ;
//  uint64_t dfreq = DAC_freq;
//  result = (dfreq*EP*256)/sfreq;
//  
//
////  if (EP > 512) // >512
////    result = (uint32_t)(((uint64_t)DAC_freq*EP))/SYSTEM_FREQ *256;
////  else
////    result = (uint32_t)((uint64_t)DAC_freq*EP*256)/SYSTEM_FREQ;
  result = coarse*256+fine;
  log_3("coarse:%hu, fine:%hu, period:%d",coarse,fine,result);
  return result;
}

uint8_t AYEnvTypToReg(uint8_t t){
  uint8_t typ_table[] = {0, 4, 11, 13, 8, 12, 14, 10};
  return typ_table[t];
}

#define AYRegToNoise(noise_period) ((uint32_t)DAC_freq*(noise_period & 31)*16)/SYSTEM_FREQ

//#define AYRegToVol(amplitude) { \
//  levels[amplitude] }
////  ((amplitude+1)*(PULSE_HIGH+1) /16)-1  }

uint16_t freqToPeriod(uint16_t freq){
  return (SYSTEM_FREQ / freq +8) / 16;  
}

void AY_set_tone(uint8_t ch, uint16_t tone_freq) {
  noInterrupts();
  AY_config[ch].tone_freq = tone_freq;
  AY_config[ch].max_count = DAC_freq()/2; // /2;
  AY_config[ch].tone_level = 0;
  AY_config[ch].counter = 0;
  AY_config[ch].tone_enabled = true;
  //------------------------
  //if (AY_config[ch].env_enabled) AY_set_env(AY_reg[R_ENV_SHAPE],AYRegToEnv(AY_reg[R_FINE_EP],AY_reg[R_COARSE_EP]));
  interrupts();
}

void AY_reset_tone(uint8_t ch) {
  AY_config[ch].tone_enabled = false;
}
    // ENVELOPE
    // 0      \__________     single decay then off
    //
    // 4      /|_________     single attack then off
    //
    // 8      \|\|\|\|\|\     repeated decay
    //
    // 9      \__________     single decay then off
    //
    //10      \/\/\/\/\/\     repeated decay-attack
    //          _________
    //11      \|              single decay then hold
    //
    //12      /|/|/|/|/|/     repeated attack
    //         __________
    //13      /               single attack then hold
    //
    //14      /\/\/\/\/\/     repeated attack-decay
    //
    //15      /|_________     single attack then off

void disable_env(void){
}

void single_decay_then_off(void){
  static boolean f = false;
  switch (AY_env.phase) {
    case 0: 
//      if (!f && (AY_env.level == 15)) {
//        digitalWrite(SD_LED, LED_ON);
//        f = true;
//      }
      AY_env.inc_counter+=AY_env.increment;       
      if (AY_env.inc_counter>=AY_env.pulses5){
        if (AY_env.level>0)
          AY_env.level --;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level == 0) {
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = 0;
//      digitalWrite(SD_LED, LED_OFF);
//      f = false;
      break;
  }
}
void single_attack_then_off(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        AY_env.level ++;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level > 15) {
        AY_env.level = 15;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = 0;
      break;
  }
}
void repeated_decay(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        if (AY_env.level > 0) 
          AY_env.level --;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level == 0) {
        AY_env.level = 15;
        AY_env.inc_counter = 0;
      }
      break;
  }
}
void repeated_decay_attack(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        if (AY_env.level > 0)
          AY_env.level --;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level == 0) {
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
        AY_env.level = 0;
      }
      break;
    case 1: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        AY_env.level ++;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level > 15) {
        AY_env.level = 15;
        AY_env.inc_counter = 0;
        AY_env.phase = 0;
      }
      break;
  }
}
void single_decay_then_hold(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        if (AY_env.level > 0)
          AY_env.level --;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level == 0) {
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
        AY_env.level = 0;
      }
      break;
    case 1: 
      AY_env.level = 15;
      break;
  }
}
void repeated_attack(void){
  AY_env.inc_counter+=AY_env.increment;
  if (AY_env.inc_counter>=AY_env.pulses5){
    AY_env.level ++;
    AY_env.inc_counter -= AY_env.pulses5;
  }
  if (AY_env.level > 15) {
    AY_env.level = 0;
    AY_env.inc_counter = 0;
  }
}
void single_atack_then_hold(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        AY_env.level ++;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level > 15) {
        AY_env.level = 15;
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = 15;
      break;
  }
}
void repeated_attack_decay(uint8_t ch){
  switch (AY_env.phase) {
    case 0: 
      AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        AY_env.level ++;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level > 15) {
        AY_env.level = 15;
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
     AY_env.inc_counter+=AY_env.increment;
      if (AY_env.inc_counter>=AY_env.pulses5){
        if (AY_env.level > 0)
          AY_env.level --;
        AY_env.inc_counter -= AY_env.pulses5;
      }
      if (AY_env.level == 0) {
        AY_env.inc_counter = 0;
        AY_env.phase = 0;
        AY_env.level = 0;
      }
      break;
  }
}

// FE = fclock/(256*EP)
// escalon = EP /16 seg
// escalon5 = EP*5 / 16
// pulsos de 22KHz equivalentes
// 1 pulso de 22KHz = 1/22000 seg
// p_eq = (EP/16)seg/(1/22000)seg = EP/(16*22000)

//  incrementar/decrementar volumen cada(256*EP)/1773450*22000/16 pulsos = EP*7040/35469 pulsos
// multiplicando todo por 5 ganamos precision
// npulsos5 = EP*(7040*5)/35469
// se inicializa con contador = 0; y a cada pulso hacemos: contador += 5; 
// if (contador >= npulsos5) { contador -= npulsos5; volumen++; }
void AY_set_env(uint8_t env_typ, uint16_t env_period){
  //return;
  boolean dummy = AY_env.enabled;
  //noInterrupts();
  AY_env.enabled = false;
  AY_env.phase = 0;
  AY_env.typ = env_typ;
  AY_env.period = env_period;
  AY_env.inc_counter = 0;
  if (AY_env.period>0){

    // EP(10)*256/SYSTEM_FREQ
    // pulses(22KHz) = EP(10)*(256*22000)/(16*SYS_FREQ)=EP(10)*(256*22000)/(16*1773450)=EP(10)*(352000/1773450)
    // pulses5(22KHz)= EP(10)*(352000/354690) 
//    AY_env.pulses5 = (uint64_t) env_period*(uint64_t)352000/(uint64_t)354690;//*35200/35469; 
//    AY_env.increment = 5;
    AY_env.pulses5 = (uint64_t) env_period*(uint64_t)35200/(uint64_t)35469;//*35200/35469; 
    AY_env.increment = 5;


    switch (AY_env.typ){
      case 0:
      case 1:
      case 2:
      case 3:
      case 9:
        AY_env.level = 15;
        AY_env.handle = &single_decay_then_off;
        log_3("Single decay: (%hu) pulses %lu period: %d",env_typ, AY_env.pulses5, AY_env.period);
        break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 15:
        AY_env.handle = &single_attack_then_off;
        AY_env.level = 0;
        log_3("Single attack_then_off: (%d) pulses %d period: %d",env_typ, AY_env.pulses5, AY_env.period);
        break;
      case 8:
        AY_env.handle = &repeated_decay;
        AY_env.level = 15;
        log_3("repeated decay: (%d) pulses %d period: %d",env_typ, AY_env.pulses5, AY_env.period);
        break;
      case 10:
        AY_env.handle = &repeated_decay_attack;
        AY_env.level = 15;
        log_3("repeated_decay_attack: (%d) pulses %d period: %d",env_typ, AY_env.pulses5, AY_env.period);
        break;
      case 11:
        AY_env.handle = &single_decay_then_hold;
        AY_env.level = 15;
        break;
      case 12:
        log_3("repeated_attack: (%hu) pulses %lu period: %d",env_typ, AY_env.pulses5, AY_env.period);
        AY_env.handle = &repeated_attack;
        AY_env.level = 0;
        break;
      case 13:
        log_3("single_atack_then_hold: (%hu) pulses %lu period: %d",env_typ, AY_env.pulses5, AY_env.period);
        AY_env.handle = &single_atack_then_hold;
        AY_env.level = 0;
        break;
      case 14:
        log_3("repeated_attack_decay: (%hu) pulses %lu period: %d",env_typ, AY_env.pulses5, AY_env.period);

        AY_env.handle = &repeated_attack_decay;
        AY_env.level = 0;
        break;
      default:
        log_3("???single_atack_then_hold: (%d) pulses %d period: %d",env_typ, AY_env.pulses5, AY_env.period);
        AY_env.handle = &single_atack_then_hold;
        AY_env.level = 0;
        break;
    }

  } else {
    AY_env.period = 0;
  }
  AY_env.inc_counter = 0;
  AY_env.enabled = dummy;
  //interrupts();
}

void AY_set_tone_env(uint8_t ch){
  AY_config[ch].env_enabled = true;
  AY_env.enabled = true;
   log_3("env enabled, ch=%hu",ch);
}
void AY_reset_tone_env(uint8_t ch){
  AY_config[ch].env_enabled = false;
  if (!AY_config[0].env_enabled & !AY_config[1].env_enabled & !AY_config[2].env_enabled) AY_env.enabled = false;
  // log_3("env enabled, ch=%hu",ch);
}

void AY_set_noise(uint8_t ch, uint16_t noise_period){
  AY_config[ch].noise_enabled = true;
  AY_config[ch].noise_counter = 0;
  AY_config[ch].rng = 1;
}

void AY_reset_noise(uint8_t ch){
  AY_config[ch].noise_enabled = false;
}

void AY_init(void){
  AY_env.period = 0;
  AY_env.level = 15;
  AY_env.inc_counter = 0;
  AY_env.typ = 0;
  for (int i=0; i<3; i++)
  {
    AY_config[i].env_enabled = false;
    AY_config[i].counter = 0;
    AY_config[i].noise_level = 0;
    AY_config[i].noise_counter = 0;
    AY_config[i].rng = 0;
    AY_config[i].tone_level = 0;
    AY_config[i].tone_freq = DAC_freq;
//    AY_config[i].tone_tics = 0;
    AY_config[i].tone_freq = DAC_freq();
    AY_config[i].tone_enabled = false;
    AY_config[i].tone_period = 0;
    AY_config[i].volume = PULSE_HIGH;
  }
  AY_set_env(14,200);

}

const uint16_t freq_t []PROGMEM = {
     16,    17,    18,    19,    20,    21,    23,    24,    26,    27,    29,    31,
     33,    35,    37,    39,    41,    43,    46,    49,    52,    55,    58,    62,
     65,    69,    73,    77,    82,    87,    92,    98,   104,   110,   117,   124,
    131,   139,   147,   156,   165,   175,   185,   196,   208,   220,   233,   247,
    262,   277,   294,   311,   330,   349,   370,   392,   415,   440,   466,   494,
    523,   554,   587,   622,   659,   698,   740,   784,   831,   880,   932,   988,
   1047,  1109,  1175,  1245,  1319,  1397,  1480,  1568,  1661,  1760,  1865,  1976,
   2093,  2217,  2349,  2489,  2637,  2794,  2960,  3136,  3322,  3520,  3729,  3951,
   4186,  4435,  4699,  4978,  5274,  5587,  5920,  6272,  6645,  7040,  7459,  7902,
   8372,  8869,  9397,  9956, 10548, 11175, 11839, 12543, 13289, 14080, 14917, 15804
  };
uint16_t note_freq(uint8_t octave, uint8_t note){
  if (octave>9) octave = 9;
  uint16_t v = pgm_read_word (freq_t+(octave-1)*12+note);
  return v;
}

void AY_set_volume(uint8_t ch, uint8_t volume){
  AY_config[ch].volume = levels[volume];
  switch (ch) {
    case 0:
      AY_reg[R_AMPLITUDE_A] |= volume;
      break;
    case 1:
      AY_reg[R_AMPLITUDE_B] |= volume;
      break;
    case 2:
      AY_reg[R_AMPLITUDE_C] |= volume;
      break;
  }
}

void AY_start(void){
  DAC_AY_active = true;
}

void AY_stop(uint8_t ch){
  AY_config[ch].tone_enabled = false;
}

void AY_stop(void){
  DAC_AY_active = false;
  for (int i=0; i<3; i++)
  {
    AY_config[i].tone_enabled = false;
  }
}

void AY_synthetize(void){
  uint16_t pulse_env;
  uint8_t noise;
  if (AY_env.enabled){
    (*AY_env.handle)();
  }
  if (AY_env.level > 15) AY_env.level = 15;
  pulse_env = levels[AY_env.level];
  for (uint8_t i=0; i<3; i++)
  {
    if (AY_config[i].tone_enabled) {
      AY_config[i].counter+=AY_config[i].tone_freq;
      if (AY_config[i].counter>=AY_config[i].max_count){
        AY_config[i].signal = !AY_config[i].signal;
        AY_config[i].counter -= AY_config[i].max_count;
      }

      if (AY_config[i].signal||AY_config[i].env_enabled) {
          //troggle_status_LED();
        if (AY_config[i].env_enabled) {
          AY_config[i].tone_level = pulse_env;
        } else
          AY_config[i].tone_level = AY_config[i].volume;        
      } else AY_config[i].tone_level = 0;

      if (AY_config[i].noise_enabled){
        // NOISE
        AY_config[i].noise_counter++;
        if (AY_config[i].noise_counter >= AY_config[i].noise_period){
          if (((AY_config[i].rng + 1) & 0x02) != 0)
             AY_config[i].noise_level = !AY_config[i].noise_level;
          if ((AY_config[i].rng & 0x01) != 0) 
             AY_config[i].rng = AY_config[i].rng ^ 0b100100000000000000;  // $24000
          AY_config[i].rng = AY_config[i].rng >> 1;
          if (AY_config[i].noise_level)
             AY_config[i].tone_level = AY_config[i].volume; 
          AY_config[i].noise_counter = 0;
        }
      }

      DAC_AY_data[i] = AY_config[i].tone_level;
    } else 
      DAC_AY_data[i] = 0;
  }
}

uint32_t note_duration(uint8_t tempo, uint8_t noteLen){
  return (uint32_t) 1000*note_len[noteLen-1] / (tempo*2);
}

uint8_t env;
int32_t env_period;
int modif[3] = {0,0,0};
int cur_octave[3] = {4,4,4};
int32_t cur_delay[3] = {250,250,250};
uint32_t cur_tempo = 120;
int index[3] = {0,0,0};
uint64_t ini_time[3] = {0,0,0};
int snote_len[3] = {0,0,0};
int noteLen[3][2] = {{5,5},{5,5},{5,5}};
char* s[3];
bool nota[3];
uint8_t AY_brackets_start[3][20];
uint8_t AY_brackets_times[3][20];
int AY_brackets_index[3] ={-1,-1,-1};
bool octave[3] = {false,false,false};
uint8_t volCh[3] = {15,15,15};

uint16_t get_num(char* s, int* i){
  char num[6];
  int j;
  for (j=(*i); isdigit(s[j]); j++);
  int len = j-(*i);
  if (len > 0) {
    memcpy(num,s+(*i),len);
    (*i) = j-1;
    return atoi(num);
  } else return -1;
}

void skip_comments(char* s, int* i){
  char num[6];
  int j;
  for (j=(*i); (s[j]!=':') && (s[j]!=0); j++);
  (*i) = j;
}

void AY_SetReg(uint8_t reg, uint8_t value){
  if (reg <= 015) AY_reg[reg] = value;

  switch (reg) {
    case R_FINE_TUNE_A:
    case R_COARSE_TUNE_A:
      AY_set_tone(0,AYRegToTone(AY_reg[R_FINE_TUNE_A], AY_reg[R_COARSE_TUNE_A]));
      break;
    case R_FINE_TUNE_B:
    case R_COARSE_TUNE_B:
      AY_set_tone(1,AYRegToTone(AY_reg[R_FINE_TUNE_B], AY_reg[R_COARSE_TUNE_B]));
      break;
    case R_FINE_TUNE_C:
    case R_COARSE_TUNE_C:
      AY_set_tone(2,AYRegToTone(AY_reg[R_FINE_TUNE_C], AY_reg[R_COARSE_TUNE_C]));
      break;
    case R_NOISE_PERIOD:
      AYRegToNoise(AY_reg[R_NOISE_PERIOD]);
      break;
    case R_ENABLE: 
      if (bitRead(AY_reg[R_ENABLE],0)==0) AY_config[AY_CH_A].tone_enabled = true; else AY_reset_tone(AY_CH_A);
      if (bitRead(AY_reg[R_ENABLE],1)==0) AY_config[AY_CH_B].tone_enabled = true; else AY_reset_tone(AY_CH_B);
      if (bitRead(AY_reg[R_ENABLE],2)==0) AY_config[AY_CH_C].tone_enabled = true; else AY_reset_tone(AY_CH_C);
      if (bitRead(AY_reg[R_ENABLE],3)==0) AY_set_noise(AY_CH_A, AYRegToNoise(AY_reg[R_NOISE_PERIOD])); else AY_reset_noise(AY_CH_A);
      if (bitRead(AY_reg[R_ENABLE],4)==0) AY_set_noise(AY_CH_B, AYRegToNoise(AY_reg[R_NOISE_PERIOD])); else AY_reset_noise(AY_CH_B);
      if (bitRead(AY_reg[R_ENABLE],5)==0) AY_set_noise(AY_CH_C, AYRegToNoise(AY_reg[R_NOISE_PERIOD])); else AY_reset_noise(AY_CH_C);
      break;
    case R_AMPLITUDE_A:
      // if (AY_reg[R_AMPLITUDE_A]>15)
      //   log_0("A.amp=%hhu",AY_reg[R_AMPLITUDE_A]);
      if (bitRead(AY_reg[R_AMPLITUDE_A],4)==1) AY_set_tone_env(AY_CH_A); else AY_reset_tone_env(AY_CH_A);
      AY_set_volume(AY_CH_A, AY_reg[R_AMPLITUDE_A] & 0x0F);
      break;
    case R_AMPLITUDE_B:
      // if (AY_reg[R_AMPLITUDE_B]>15)
      //   log_0("B.amp=%hhu",AY_reg[R_AMPLITUDE_B]);
      if (bitRead(AY_reg[R_AMPLITUDE_B],4)==1) AY_set_tone_env(AY_CH_B); else AY_reset_tone_env(AY_CH_B);
      AY_set_volume(AY_CH_B, AY_reg[R_AMPLITUDE_B] & 0x0F);
      break;
    case R_AMPLITUDE_C:
      // if (AY_reg[R_AMPLITUDE_C]>15)
      //   log_0("C.amp=%hhu",AY_reg[R_AMPLITUDE_C]);
      if (bitRead(AY_reg[R_AMPLITUDE_C],4)==1) AY_set_tone_env(AY_CH_C); else AY_reset_tone_env(AY_CH_C);
      AY_set_volume(AY_CH_C, AY_reg[R_AMPLITUDE_C] & 0x0F);
      break;
    case R_FINE_EP:
    case R_COARSE_EP:
    case R_ENV_SHAPE:
      uint16_t period = AYRegToEnv(AY_reg[R_FINE_EP],AY_reg[R_COARSE_EP]);
      log_3("SHAPE=%hu, FINE=%hhu, COARSE=%hhu, PERIOD=%d",AY_reg[R_ENV_SHAPE],AY_reg[R_FINE_EP],AY_reg[R_COARSE_EP],period);
      AY_set_env(AY_reg[R_ENV_SHAPE],period);
      //log_0("A=%hhu, B=%hhu, C=%hhu",AY_config[0].env_enabled,AY_config[1].env_enabled,AY_config[2].env_enabled);      
      break;
  }
}

bool finished[3]={false,false,false};
int16_t Play_AY_channel(char* s, int channel){
  int i;
  int vol = 15;
  uint8_t value;
  uint8_t oct;
  uint8_t c;
  uint8_t l;
  uint8_t l2;
  uint16_t env_period;
  uint16_t tone_period;
  uint16_t dummy;
  uint8_t dummy1;
  uint8_t dummy2;
  
  i = index[channel];
  if (!nota[channel]){
    if (i < snote_len[channel]) {
      switch (s[i]){
        case '$': modif[channel] = -1;
                  break;
        case '=': modif[channel] = +1;
                  break;
        case '&': // silencios  
                  // AY_SetReg(RegFTune[channel],0);
                  // AY_SetReg(RegCTune[channel],0);
                  AY_SetReg(RegVol[channel],0);
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break;
        case 'v': 
        case 'V': // volumen (0..15)
                  i++;
                  vol = get_num(s,&i);
                  if ((vol < 0) || (vol > 15)) return 9;
                  volCh[channel] = vol;
                  AY_SetReg(RegVol[channel],volCh[channel]);
                  break;
        case ' ': 
        case 'n': 
        case 'N': // dummy note 
                  break;
        case 'm':
        case 'M': 
                  i++; 
                  value = get_num(s,&i);
                  AY_SetReg(7, 63 - value);
                  break;
        case 't': 
        case 'T': // tempo (negras por minuto)(60..240) 
                  i++; 
                  cur_tempo = get_num(s,&i);
                  if ((cur_tempo <60) || (cur_tempo > 240)) return 9;
                  for (c=0; c<3; c++){
                    if (noteLen[c][2]>12)
                      cur_delay[c] = note_duration(cur_tempo, noteLen[c][0]);
                    else
                      cur_delay[c] = note_duration(cur_tempo, noteLen[c][0])+note_duration(cur_tempo, noteLen[c][1]);
                  }
                  break;
        case 'h': 
        case 'H': // STOP inmediately
                  finished[0]=finished[1]=finished[2]=true;
                  break;
        case 'w': 
        case 'W': // Envelope (0..7)
                  i++;
                  env = get_num(s,&i);
                  if ((env > 7) || (env <0)) return 9;
                  AY_SetReg(R_ENV_SHAPE, AYEnvTypToReg(env));
                  break;
        case 'x': 
        case 'X': // Envelope len(0..65535)
                  i++;
                  env_period = get_num(s,&i);
                  AY_SetReg(R_FINE_EP, (uint8_t) env_period & 0xff); 
                  AY_SetReg(R_COARSE_EP, (uint8_t) (env_period >> 8) & 0xff); 
                  break;
        case 'u':
        case 'U': AY_SetReg(RegVol[channel],RegVol[channel] | B10000);              
                  break;
        case ':': // ignored notes :...:
                  i++;
                  skip_comments(s,&i);
                  break;
        case '(': // repeated notes start (...)
                  AY_brackets_index[channel]++;
                  AY_brackets_start[channel][AY_brackets_index[channel]]=i;
                  AY_brackets_times[channel][AY_brackets_index[channel]] = 0;
                  break;
        case ')': // repeated notes end(...)
                   if (AY_brackets_index[channel] < 0) { // Repeat forever
                     i = -1;
                  } else
                  if (AY_brackets_times[channel][AY_brackets_index[channel]] < 1) {
                    AY_brackets_times[channel][AY_brackets_index[channel]]++;
                    i = AY_brackets_start[channel][AY_brackets_index[channel]];
                  }else {
                    if (AY_brackets_index[channel] >= 0) 
                      AY_brackets_index[channel]--;
                  }
                  break;
        case 'o': 
        case 'O': i++;
                  oct = get_num(s,&i); 
                  if ((oct < '9')) cur_octave[channel] = oct;
                  else return 10; 
                  break;
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8': 
        case '9': 
              l = get_num(s,&i);
              if (l > 12) return 9;
              if (s[i+1]=='-') {              // tied note
                i++;
                l2 = get_num(s,&i);
                noteLen[channel][0] = l;
                noteLen[channel][1] = l2;
                cur_delay[channel] = note_duration(cur_tempo,noteLen[channel][0])+note_duration(cur_tempo,noteLen[channel][1]);
              } else {
                noteLen[channel][0] = l;
                noteLen[channel][1] = 255;
                cur_delay[channel] = note_duration(cur_tempo,noteLen[channel][0]);
              }
              break;
        case 'c': 
        case 'd': 
        case 'e': 
        case 'f': 
        case 'g': 
        case 'a': 
        case 'b': 
                  tone_period = freqToPeriod(note_freq(cur_octave[channel],whiteKeys[s[i]-'a']+modif[channel]));
                  AY_SetReg(rfine[channel],tone_period & 0xff);
                  AY_SetReg(rcoarse[channel],(tone_period >> 8) & 0xff);
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  AY_SetReg(RegVol[channel],volCh[channel]);
                  break; 
        case 'C': 
        case 'D': 
        case 'E': 
        case 'F': 
        case 'G': 
        case 'A': 
        case 'B': 
                  tone_period = freqToPeriod(note_freq(cur_octave[channel]+1,whiteKeys[s[i]-'A']+modif[channel]));
                  AY_SetReg(rfine[channel],tone_period & 0xff);
                  AY_SetReg(rcoarse[channel],(tone_period >> 8) & 0xff);
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  AY_SetReg(RegVol[channel],volCh[channel]);
                  break; 
        case '\0': break;
        default: 
          return 10;
      }
    }    
  }
  if (nota[channel]){
    uint32_t t = millis()-ini_time[channel];
    if (t > cur_delay[channel]) {
      i++;
      nota[channel] = false;
      AY_set_tone(channel,0); // mute channel
    }
  } else i++;
  index[channel] = i;
  if (i >= snote_len[channel]) finished[channel]= true;
  return 0;
}

void init_AY_registers(){
  AY_SetReg(R_FINE_TUNE_A,0);
  AY_SetReg(R_COARSE_TUNE_A,0);
  AY_SetReg(R_FINE_TUNE_B,0);
  AY_SetReg(R_COARSE_TUNE_B,0);
  AY_SetReg(R_FINE_TUNE_C,0);
  AY_SetReg(R_COARSE_TUNE_C,0);
  AY_SetReg(R_NOISE_PERIOD,0);
  AY_SetReg(R_ENABLE,B111000);
  AY_SetReg(R_AMPLITUDE_A,15);
  AY_SetReg(R_AMPLITUDE_B,15);
  AY_SetReg(R_AMPLITUDE_C,15);
  AY_SetReg(R_FINE_EP,0);
  AY_SetReg(R_COARSE_EP,0);
  AY_SetReg(R_ENV_SHAPE,0);

}

int16_t Play_AY(char* p1, char* p2, char* p3){
  cur_delay[0] = note_duration(cur_tempo, noteLen[0][0]);
  cur_delay[1] = note_duration(cur_tempo, noteLen[1][0]);
  cur_delay[2] = note_duration(cur_tempo, noteLen[2][0]);
  nota[0]=nota[1]=nota[2]=false;
  s[0]=p1;
  s[1]=p2;
  s[2]=p3;

  init_AY_registers();
  
  index[0]=index[1]=index[2]=0;
  finished[0]=finished[1]=finished[2]=false;
  snote_len[0]=strlen(s[0]);
  snote_len[1]=strlen(s[1]);
  snote_len[2]=strlen(s[2]);
  AY_reset_noise(0);
  AY_start();
  while (!finished[0] || !finished[1] || !finished[2]) {
    for (int channel=0;channel<3;channel++){
      if (!finished[channel]){
        uint8_t result = Play_AY_channel(s[channel], channel);
        if (result != 0) {
          AY_stop();
          return result;
        }
      }
    }
  }
  AY_stop();
  return 0;
}
