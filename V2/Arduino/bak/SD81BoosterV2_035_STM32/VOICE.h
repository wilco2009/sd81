/*********************************************************/

SdFile fonema_file;
//SdFile wav_File;
#define MAX_ALOFONE 60
//byte v[] = {0x80,0x8B,0x87,0xAD,0xB5,0x82, 0x8D, 0x93, 0x80, 0xC0, 0x80, 0xC1, 0x80, 0xC2, 0x80, 0xE8, 0x80}; // HELLO
byte v[MAX_ALOFONE];
char audioFile [100];
int cur_alofone = 0;
int alofone_write_pos = 0;
byte voice_bank = 0xE4;
//byte voice_bank = 0xE8;
long playing_pos = 0;
int alofone_index = -1;
#define serial_alofone_queue_size 100
char serial_alofones [serial_alofone_queue_size] = "";
int serial_alofones_index = 0;

/*********************************************************/

void voice_init() {
  
  log_0("VOICE EMULATOR Begin");
  serial_alofones[0] = 0;
  serial_alofones_index = 0;
  
}

byte HEX2BYTE(char h){
  if ((h >= '0') && (h <= '9')) return h-'0';
  else if (h >= 'a') return 10+h-'a';
  else return 10+h-'A';
}

byte read_alofone_from_serial(void){
  char d1;
  char d2;
  byte v1;
  byte v2;

  if (strlen(serial_alofones)> serial_alofones_index) {
    d1 = serial_alofones[serial_alofones_index];
    d2 = serial_alofones[serial_alofones_index+1];
  }
  serial_alofones_index+=2;
  if(serial_alofones[serial_alofones_index]==' ') serial_alofones_index++;
  v1 = HEX2BYTE(d1);
  v2 = HEX2BYTE(d2);
  return v1*16+v2;
}

void open_fonema_file(void){
  if (fonema_file.isOpen())
    fonema_file.close();
  fonema_file.open("/SYS/VOICE/ALOFONES.DAT");
  alofone_index = 0;
  voice_bank = 0xE4;
}

long alofone_start_position(byte bank, byte alofone){
  return pgm_read_dword(alof_pos+(long)(bank-0xE4)*0x80+(long)(alofone-0x80)/**sizeof(long int)*/);
}

long alofone_size(byte bank, byte alofone){
  return alofone_start_position(bank, alofone+1)-alofone_start_position(bank, alofone);
}

long alofone_end_position(byte bank, byte alofone){
  return alofone_start_position(bank, alofone)+alofone_size(bank, alofone);
}

boolean is_bank(byte alofone){
  return alofone >= 0xE0 && alofone <= 0xEF;
}

byte next_alofone(void){
int i;
  if (alofone_index == alofone_write_pos){
    return 0;
  } else {
    if (!is_bank(v[alofone_index])){
      i = v[alofone_index];
      alofone_index++;
    } else {
      voice_bank = v[alofone_index];
      alofone_index++;
      i = next_alofone();
    }
    if (alofone_index>=MAX_ALOFONE) alofone_index = 0;
    return i;
  }
}

bool alofone_queue_empty(void){
  return (alofone_index == alofone_write_pos);
}

bool alofone_queue_full(){
  int i = alofone_write_pos+1;
  if (i>=MAX_ALOFONE) 
    i = 0;
  if (i == alofone_index) {
    return true;
  } else {
    return false;
  }  
}

bool add_alofone(byte alofone){
  int i = alofone_write_pos+1;
  if (i>=MAX_ALOFONE) 
    i = 0;
  if (i != alofone_index) {
    v[alofone_write_pos] = alofone;
    alofone_write_pos = i;
    return true;
  } else {
    return false;
  }
}

bool playing_alofone(void){
  return (cur_alofone != 0) && (playing_pos >= alofone_start_position(voice_bank, cur_alofone)) &&(playing_pos<alofone_end_position(voice_bank, cur_alofone));
}

byte get_alofone_byte(long pos){
  if (cur_alofone==0) return 0;
  else {
    fonema_file.seekSet(pos);
    return fonema_file.read();
  }
}

void show_alofone_buffer(void){
}

void open_next_alofone(void){
  if (!playing_alofone()){
    cur_alofone = next_alofone(); 
    playing_pos = alofone_start_position(voice_bank, cur_alofone)+44;
//    Serial.print("ALOFONE="); Serial.println(cur_alofone);
//  char str1[100];
//    sprintf_P(str1, PSTR("BANK:%02X AL%02X P:%lu SP:%lu Size:%d EP:%lu "), voice_bank, cur_alofone, playing_pos, alofone_start_position(voice_bank, cur_alofone), alofone_size(voice_bank, cur_alofone), alofone_end_position(voice_bank, cur_alofone));
//    Serial.println(str1);
  }
}

bool led = false;

  char str1[100];

void showHex(unsigned char* p){
  sprintf_P(str1, PSTR("BANK:%02X AL:%02X P:%lu SP:%lu Size:%d EP:%lu "), voice_bank, cur_alofone, playing_pos, alofone_start_position(voice_bank, cur_alofone), alofone_size(voice_bank, cur_alofone), alofone_end_position(voice_bank, cur_alofone));
  Serial.println(str1);
  for (int k=0; k < 100; k+=0x0F){
    sprintf_P(str1, PSTR("%04X "),k);
    Serial.print(str1);
    for (int j=0; j<0x10; j++){
      sprintf_P(str1, PSTR("%02x "),p[k+j]);
      Serial.print(str1);
    }
    Serial.println();
  }
}

void load_alofone_buffer(){
  //Load samples into the 8-bit dacBuffer
  if(!playing_alofone()){
    open_next_alofone();
  }
  if (cur_alofone==0) return;
  long bytes_left = (alofone_end_position(voice_bank, cur_alofone)-playing_pos)+1;
  #ifdef LOG3
  sprintf_P(str1, PSTR("BYTES LEFT: %lu, BUFFER SIZE: %d"), bytes_left, DAC_BUFFER_SIZE);
  Serial.println(str1);
  #endif
  if (bytes_left > DAC_BUFFER_SIZE){
    fonema_file.seekSet(playing_pos);
    fonema_file.read((void*) DAC_work_buffer+ DAC_work_buffer_count, DAC_BUFFER_SIZE);
    #ifdef LOG3
    showHex(DAC_work_buffer);
    #endif
    playing_pos+= DAC_BUFFER_SIZE;
    DAC_work_buffer_count+= DAC_BUFFER_SIZE;
  } else {
    fonema_file.seekSet(playing_pos);
    uint32_t result = fonema_file.read((void*) DAC_work_buffer+ DAC_work_buffer_count, bytes_left);
    #ifdef LOG3
    Serial.println(result);
    showHex(DAC_work_buffer);
    #endif
    playing_pos+= bytes_left;
    DAC_work_buffer_count+= bytes_left;
  }
  DAC_feed();
}

void play_next_alofone(void){
  open_next_alofone();
  load_alofone_buffer();
}

void init_alofones(void){
  voice_bank = 0xE4;
  cur_alofone = 0;
  alofone_index = 0;
  alofone_index = 0;
  alofone_write_pos = 0;
}

void playAudio(void){

  open_fonema_file();
  do {
    #ifdef LOG3
    Serial.println(cur_alofone);
    #endif
    play_next_alofone();
  } while (!alofone_queue_empty());
}

void voiceTest(void){
  byte VOICE_EM [] = {0xE4, 0x80, 0xB6, 0x87, 0x81, 0xBF, 0x98, 0x8C, 0xB7, 0xB7, 0x83, 0x87, 0x90, 0x96, 0xAD, 0x94, 0xA7, 0x80, 0x00};  
    memcpy(v, VOICE_EM, sizeof(VOICE_EM));
  cur_alofone = 0;
  alofone_index = 0;
  alofone_write_pos = sizeof(VOICE_EM);
  
  playAudio();

}
