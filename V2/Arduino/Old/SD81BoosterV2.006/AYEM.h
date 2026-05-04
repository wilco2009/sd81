//#define SYSTEM_FREQ 1773400
#define SYSTEM_FREQ 2000000    
#define MIN_PULSE_HIGH 32

#define AY_CH_A 0
#define AY_CH_B 1
#define AY_CH_C 2

uint8_t AY_reg[16]= {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};

//  f=reloj/(v*16)
 

#define PULSE_HIGH 255
#define PULSE_LOW 0

uint8_t levels[16] = {0,31,47,63,79,95,111,127,143,159,175,191,207,223,239,255};
//uint16_t OCTAVE8[12] = {4186, 4435, 4699, 4978, 5274, 5588, 5920, 6272, 6645, 7040, 7459, 7902};

// duracion en negras multiplicado por 8
uint16_t note_len[12] = {30,45,60,90,120,180,240,360,480,20,40,80};

#define AYRegToTone(fine, coarse){ \
  SYSTEM_FREQ / ((uint32_t)coarse*256+fine)/16  \ 
}

uint32_t AYRegToEnv(uint8_t fine, uint8_t coarse){
  uint32_t result;
  uint32_t EP = (uint16_t)(coarse*256)+fine;

  if (EP > 512) // >512
    result = (uint32_t)(((uint64_t)DAC_freq*EP))/SYSTEM_FREQ *256;
  else
    result = (uint32_t)((uint64_t)DAC_freq*EP*256)/SYSTEM_FREQ;
  return result;
}

#define AYRegToNoise(noise_period) { \
  ((uint32_t)DAC_freq*(noise_period & 31)*16)/SYSTEM_FREQ \
}

#define AYRegToVol(amplitude) { \
  levels[amplitude] }
//  ((amplitude+1)*(PULSE_HIGH+1) /16)-1  }

void AY_set_tone(uint8_t ch, uint16_t tone_freq) {
  AY_config[ch].tone_freq = tone_freq;
  AY_config[ch].max_count = DAC_freq(); // /2;
  AY_config[ch].tone_enabled = true;
  AY_config[ch].tone_level = 0;
  AY_config[ch].counter = 0;
  Serial.println(tone_freq);
  
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


void single_decay_then_off(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level --;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = PULSE_LOW;
      break;
  }
}
void single_attack_then_off(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level ++;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = PULSE_LOW;
      break;
  }
}
void repeated_decay(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level --;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.level = PULSE_HIGH;
        AY_env.inc_counter = 0;
      }
      break;
  }
}
void repeated_decay_attack(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level --;
        if (AY_env.level < 0) AY_env.level = 0;
        AY_env.inc_counter = 0;
//  char str1[100];
//  sprintf_P(str1, PSTR("PERIOD: %d, DAC: %d, FASE: %d, COUNTER: %d, VALUE: %d, INC: %d, PULSES: %d"), AY_env.period, DAC_freq(), AY_env.phase, AY_env.counter, AY_env.level, AY_env.increment, AY_env.pulses);
//  Serial.println(str1);
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
        AY_env.level = MIN_PULSE_HIGH;
      }
      break;
    case 1: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level ++;
        if (AY_env.level > 255) AY_env.level = 255;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.inc_counter = 0;
        AY_env.phase = 0;
      }
      break;
  }
}
void single_decay_then_hold(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level --;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = PULSE_HIGH;
      break;
  }
}
void repeated_attack(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level ++;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.level = PULSE_LOW;
        AY_env.inc_counter = 0;
      }
      break;
  }
}
void single_atack_then_hold(void){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level ++;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.level = PULSE_HIGH;
      break;
  }
}
void repeated_attack_decay(uint8_t ch){
  switch (AY_env.phase) {
    case 0: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level ++;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.inc_counter = 0;
        AY_env.phase = 1;
      }
      break;
    case 1: 
      AY_env.counter++;
      AY_env.inc_counter++;
      if (AY_env.inc_counter>=AY_env.increment){
        AY_env.level --;
        AY_env.inc_counter = 0;
      }
      if (AY_env.counter >= AY_env.pulses) {
        AY_env.counter = 0;
        AY_env.inc_counter = 0;
        AY_env.phase = 0;
        AY_env.level = MIN_PULSE_HIGH;
      }
      break;
  }
}


void AY_set_env(uint8_t env_typ, uint16_t env_period){
  
  AY_env.enabled = true;
  AY_env.phase = 0;
  AY_env.typ = env_typ;
  AY_env.period = env_period;
  AY_env.inc_counter = 0;
  if (AY_env.period>0){

    // EP*256/SYSTEM_FREQ
    AY_env.pulses = env_period; // env_period*DAC_freq();
    AY_env.increment = AY_env.pulses/(256-MIN_PULSE_HIGH);


    switch (AY_env.typ){
      case 0:
      case 1:
      case 2:
      case 3:
      case 9:
        AY_env.handle = &single_decay_then_off;
        AY_env.level = PULSE_HIGH;
        break;
      case 4:
      case 5:
      case 6:
      case 7:
      case 15:
        AY_env.handle = &single_attack_then_off;
        AY_env.level = PULSE_LOW;
        break;
      case 8:
        AY_env.handle = &repeated_decay;
        AY_env.level = PULSE_HIGH;
        break;
      case 10:
        AY_env.handle = &repeated_decay_attack;
        AY_env.level = PULSE_HIGH;
        break;
      case 11:
        AY_env.handle = &single_decay_then_hold;
        AY_env.level = PULSE_HIGH;
        break;
      case 12:
        AY_env.handle = &repeated_attack;
        AY_env.level = PULSE_LOW;
        break;
      case 13:
        AY_env.handle = &single_atack_then_hold;
        AY_env.level = PULSE_LOW;
        break;
      case 14:
        AY_env.handle = &repeated_attack_decay;
        AY_env.level = PULSE_LOW;
        break;
      default:
        AY_env.handle = &single_atack_then_hold;
        AY_env.level = PULSE_LOW;
        break;
    }

  } else {
    AY_env.period = 0;
  }
  AY_env.signal = false;
  AY_env.starting = true;
  AY_env.counter = 0;
}

void AY_set_tone_env(uint8_t ch){
  AY_config[ch].env_enabled = true;
}
void AY_reset_tone_env(uint8_t ch){
  AY_config[ch].env_enabled = false;
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
  AY_env.level = PULSE_HIGH;
  AY_env.counter = 0;
  AY_env.signal = false;
  AY_env.typ = 0;
  AY_env.starting = false;
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
  uint16_t v = pgm_read_word (freq_t+(octave)*12+note);
//  Serial.println(v);
  return v;
}

void AY_set_volume(uint8_t ch, uint8_t volume){
  AY_config[ch].volume = volume;
}

void AY_run(void){
  if (bitRead(AY_reg[7],0)==0) AY_set_tone(AY_CH_A, AYRegToTone(AY_reg[0],AY_reg[1])); else AY_reset_tone(AY_CH_A);
  if (bitRead(AY_reg[7],1)==0) AY_set_tone(AY_CH_B, AYRegToTone(AY_reg[2],AY_reg[3])); else AY_reset_tone(AY_CH_B);
  if (bitRead(AY_reg[7],2)==0) AY_set_tone(AY_CH_C, AYRegToTone(AY_reg[4],AY_reg[5])); else AY_reset_tone(AY_CH_C);

  if (bitRead(AY_reg[7],3)==0) AY_set_noise(AY_CH_A, AYRegToNoise(AY_reg[6])); else AY_reset_noise(AY_CH_A);
  if (bitRead(AY_reg[7],4)==0) AY_set_noise(AY_CH_B, AYRegToNoise(AY_reg[6])); else AY_reset_noise(AY_CH_B);
  if (bitRead(AY_reg[7],5)==0) AY_set_noise(AY_CH_C, AYRegToNoise(AY_reg[6])); else AY_reset_noise(AY_CH_C);

  if (bitRead(AY_reg[10],4)==0) AY_set_tone_env(AY_CH_A); else AY_reset_tone_env(AY_CH_A);
  if (bitRead(AY_reg[11],4)==0) AY_set_tone_env(AY_CH_B); else AY_reset_tone_env(AY_CH_B);
  if (bitRead(AY_reg[12],4)==0) AY_set_tone_env(AY_CH_C); else AY_reset_tone_env(AY_CH_C);
  AY_set_env(AY_reg[15],AYRegToEnv(AY_reg[13],AY_reg[14]));

  AY_set_volume(AY_CH_A, AYRegToVol(AY_reg[10] & 0x0F));
  AY_set_volume(AY_CH_B, AYRegToVol(AY_reg[11] & 0x0F));
  AY_set_volume(AY_CH_C, AYRegToVol(AY_reg[12] & 0x0F));
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
  // porqué se queda colgado??
  (*AY_env.handle)();
  pulse_env = AY_env.level;
//  if (AY_env.enabled) {
//    (*AY_env.handle)();
//    pulse_env = AY_env.level;
//  } else pulse_env = PULSE_HIGH;
  for (uint8_t i=0; i<3; i++)
  {
    if (AY_config[i].tone_enabled) {
      AY_config[i].counter+=AY_config[i].tone_freq;
      if (AY_config[i].counter>=AY_config[i].max_count){
        AY_config[i].signal = !AY_config[i].signal;
        AY_config[i].counter -= AY_config[i].max_count;
      }

      if (AY_config[i].signal) {
        if (AY_config[i].env_enabled) 
          AY_config[i].tone_level = pulse_env;
        else
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
  log_3("tempo: %d, len: %d", tempo, note_len[noteLen-1]);
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
int noteLen[3] = {5,5,5};
char* s[3];
bool nota[3];
uint8_t AY_brackets_start[3][20];
uint8_t AY_brackets_times[3][20];
int AY_brackets_index[3] ={-1,-1,-1};
bool octave[3] = {false,false,false};

uint16_t get_num(char* s, int* i){
  char num[6];
  int j;
//  Serial.println(s);
  for (j=(*i); isdigit(s[j]); j++);
//  Serial.print((*i)); Serial.print(" - "); Serial.println(j);
  int len = j-(*i);
  if (len > 0) {
    memcpy(num,s+(*i),len);
    (*i) = j-1;
    return atoi(num);
  } else return -1;
}

bool finished[3]={false,false,false};
int16_t Play_AY_channel(char* s, int channel){
  int i;
  int vol = 15;
  uint8_t oct;
  uint8_t c;
  uint8_t l;
  i = index[channel];
//  sprintf(S,"CH=%d i=%d n=%c fin=%d",channel,i,s[i],finished[i]);
//  Serial.println(S);
  if (!nota[channel]){
    log_3("i= %d s[i]=%c", i, s[i]);
    if (i < snote_len[channel]) {
      switch (s[i]){
        case '$': modif[channel] = -1;
                  break;
        case '=': modif[channel] = +1;
                  break;
        case '&': // silencios 
                   
                  AY_set_tone(channel,0);
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break;
        case 'v': 
        case 'V': // volumen por implementar (0..15)
                  i++;
                  vol = get_num(s,&i);
                  if ((vol < 0) || (vol > 15)) return 9;
                  break;
        case ' ': 
        case 'n': 
        case 'N': // dummy note por implementar
                  break;
        case 't': 
        case 'T': // tempo (negras por minuto)(60..240) por implementar
                  i++; 
                  cur_tempo = get_num(s,&i);
                  log_3("tempo=%d",cur_tempo);
                  if ((cur_tempo <60) || (cur_tempo > 240)) return 9;
                  for (c=0; c<3; c++){
                    cur_delay[c] = note_duration(cur_tempo, noteLen[c]);
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
                  Serial.println(env);
                  AY_set_env(env,env_period);
                  break;
        case 'x': 
        case 'X': // Envelope len(0..65535)
                  i++;
                  env_period = get_num(s,&i);
                  Serial.println(env_period);
                  break;
        case 'U': AY_config[channel].env_enabled = true;
                  break;
        case '-': // Tied Notes
                  break;
        case ':': // ignored notes :...:
                  break;
        case '(': // repeated notes start (...)
                  AY_brackets_index[channel]++;
                  AY_brackets_start[channel][AY_brackets_index[channel]]=i;
                  AY_brackets_times[channel][AY_brackets_index[channel]] = 0;
                  //sprintf(S, "START: i: %d, index: %d, times: %d, start: %d", i, AY_brackets_index[channel], AY_brackets_times[channel][AY_brackets_index[channel]], AY_brackets_start[channel][AY_brackets_index[channel]]);
                  //Serial.println(S);
                  break;
        case ')': // repeated notes end(...)
                  //sprintf(S, "END: i: %d, index: %d, times: %d, start: %d", i, AY_brackets_index[channel], AY_brackets_times[channel], AY_brackets_start[channel][AY_brackets_index[channel]]);
                  //Serial.println(S);
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
  //      case 'y': 
  //      case 'Y': // MIDI channel (ignored)
  //                break;
  //      case 'z': 
  //      case 'Z': // MIDI programming code (ignored)
  //                break;
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
        case '9': log_3("A len=%d i=%d",l,i);
                  l = get_num(s,&i);
                  log_3("B len=%d i=%d",l,i);
                  if (l > 12) return 9;
                  noteLen[channel] = l;
                  cur_delay[channel] = note_duration(cur_tempo,noteLen[channel]);
                  log_3("cur_delay: %lu channel: %d",cur_delay[channel], channel);
                  break;
        case 'c': AY_set_tone(channel,note_freq(cur_octave[channel],0+modif[channel]));
                  log_3("c\n\r");
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'd': AY_set_tone(channel,note_freq(cur_octave[channel],2+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'e': AY_set_tone(channel,note_freq(cur_octave[channel],4+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'f': AY_set_tone(channel,note_freq(cur_octave[channel],5+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'g': AY_set_tone(channel,note_freq(cur_octave[channel],7+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'a': AY_set_tone(channel,note_freq(cur_octave[channel],9+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'b': AY_set_tone(channel,note_freq(cur_octave[channel],11+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'C': AY_set_tone(channel,note_freq(cur_octave[channel]+1,0+modif[channel]));
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'D': AY_set_tone(channel,note_freq(cur_octave[channel]+1,2+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'E': AY_set_tone(channel,note_freq(cur_octave[channel]+1,4+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'F': AY_set_tone(channel,note_freq(cur_octave[channel]+1,5+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'G': AY_set_tone(channel,note_freq(cur_octave[channel]+1,7+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'A': AY_set_tone(channel,note_freq(cur_octave[channel]+1,9+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case 'B': AY_set_tone(channel,note_freq(cur_octave[channel]+1,11+modif[channel])); 
                  modif[channel] = 0;
                  nota[channel] = true;
                  ini_time[channel] = millis();
                  break; 
        case '\0': break;
        default: 
          log_3("default s[i]:%d\n\r",s[i]);
          return 10;
      }
    }    
  }
  if (nota[channel]){
    uint32_t t = millis()-ini_time[channel];
    if (t > cur_delay[channel]) {
      log_3("ti = %lu > %lu", t, cur_delay[channel]);
      i++;
      nota[channel] = false;
      AY_set_tone(channel,0); // mute channel
    }
  } else i++;
  index[channel] = i;
  if (i >= snote_len[channel]) finished[channel]= true;
  return 0;
}

int16_t Play_AY(char* p){
  cur_delay[0] = note_duration(cur_tempo, noteLen[0]);
  cur_delay[1] = note_duration(cur_tempo, noteLen[1]);
  cur_delay[2] = note_duration(cur_tempo, noteLen[2]);
  nota[0]=nota[1]=nota[2]=false;
  s[0]=p;
  s[1]=0;
  s[2]=0;
  int ch = 0;
  //Serial.print("PARS:");
  for (int i=0; p[i]!=0; i++){
    if (p[i]==',') {
      p[i]=0;
      ch++;
      s[ch]=p+i+1;
    }
    //Serial.print(p[i]);
  }
  //Serial.println();
  Serial.println(s[0]);
 
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
          log_3("result: %d\n\r", result);
          AY_stop();
          return result;
        }
      }
    }
  }
  AY_stop();
  return 0;
}
