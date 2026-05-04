
char ver_string[] = "SD81 VER ALFA 0.1\r"; // "SD81 VER ALFA 0.1";
#define GET_NEXT_CHAR 13

// COMMAND = 0 OK
void cmd_nop(void){
  ToggleClock();
  command_active = CMD_IDLE;
//  WAIT_DATA
  reset_commands();
}

// COMMAND = 1 OK
void cmd_ver(void){
  ToggleClock();
  SendByteToZ80(VERSION); 
  ToggleClock();
  reset_commands();
}

// COMMAND = 2 OK
void cmd_pwd(void){
uint8_t c;

  set_SDLed(LED_ON);
  ToggleClock();
  
  for (int i=0; current_dir[i]!=0;i++){
    c = GetByteFromZ80();    // GET_CHAR
    if (c!=GET_NEXT_CHAR){
      command_active = c;
      return;
    }
    SendByteToZ80(pgm_read_byte(asc_to_asc81+current_dir[i])); 
    ToggleClock();       // finish command
  }
  c = GetByteFromZ80();    // GET_CHAR
  if (c!=GET_NEXT_CHAR){
    command_active = c;
    return;
  }
  SendByteToZ80(0xFF); // EOT...
  SendByteToZ80(0x00); // ... and Status
  ToggleClock();       // Final clock toggle
  reset_commands();
}

// COMMAND = 3 OK
void cmd_cd(void){
char ss[MAX_FILENAME_LEN];
uint8_t error_code;
char file_name[MAX_FILENAME_LEN];
  error_code = 0;
  
  set_SDLed(LED_ON);
  ToggleClock();
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  //Serial.print("DIR=");Serial.println(current_dir);
  //Serial.print("LEN=");Serial.println(param_len);
  char ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  //Serial.print("PARAMS=");Serial.println(params);

  if (params[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  complete_dir(ss,tmp);
// strcpy(ss,tmp);
  //Serial.println(ss);
  dir.close();
  if (!dir.open(ss)){
    sprintf_P(tmp,PSTR("dir \"%s\" does not exists\r"),ss);
    log_0(tmp);
//    send_str(tmp);
    error_code = 1;
    strcpy(ss,current_dir);
  } else if (!dir.isDir()){
    sprintf_P(tmp,("\"%s\" is not a dir\r"),ss);
    log_0(tmp);
//    send_str(tmp);
    error_code = 2;
 //   strcpy(current_dir,ss);
  } else {
    sprintf_P(tmp,PSTR("current dir = \"%s\"\r"), ss);
    strcpy(current_dir,ss);
    // send_str(tmp);
  }
  dir.close();
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  //Serial.print("DIR=");Serial.println(current_dir);
  reset_commands();
}

// COMMAND = 4 OK
void cmd_del(){
uint8_t error_code;
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
  set_SDLed(LED_ON);
  ToggleClock();
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  //Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  char ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  complete_dir(s,current_dir);
  strcat(s,params);
  // Serial.println(s);
  error_code = 0;

  if (!sd.exists(s))
    error_code = 1;
  else if (!sd.remove(s)) 
    error_code = 3;
    
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 5 OK
void cmd_mkdir(){
uint8_t error_code;
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
  set_SDLed(LED_ON);
  error_code = 0;
  ToggleClock();

  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  //Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  char ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  if (params[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  complete_dir(s,tmp);
  
  if (sd.exists(s))
    error_code = 4;
  else if (!sd.mkdir(s)){
    sprintf_P(tmp,PSTR("can not create dir \"%s\"\r"),s);
    log_0(tmp);
    error_code = 3;
  }
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 6 OK
void cmd_rmdir(){
uint8_t error_code;
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
  set_SDLed(LED_ON);
  error_code = 0;
  ToggleClock();

  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  char ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  if (params[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  complete_dir(s,tmp);
  if (!sd.exists(s))
    error_code = 1;
  else {
    if (!sd.rmdir(s)){
      sprintf_P(tmp,PSTR("can not remove dir\r"), s);
      error_code = 3;
    }
    if (Sfile.open(s)){
      if (!Sfile.isDir()) error_code = 2;
      Sfile.close();    
    }
  } 
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 7 OK
void cmd_move(){
uint8_t error_code;
char s[MAX_FILENAME_LEN];
// char param1[MAX_PARAM_LEN];
char param2[MAX_PARAM_LEN];
char file_name[MAX_FILENAME_LEN];
  set_SDLed(LED_ON);
  error_code = 0;
  ToggleClock();

  //GET PARAMS (first param)
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAM1=");Serial.println(params);


  ToggleClock();
  //GET PARAMS (second param)
  param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    param2[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  param2[param_len] = 0;
  // Serial.print("PARAM2=");Serial.println(param2);

//  split_two_params(params, params, param2);
  if (params[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  complete_dir(params,tmp);
  
  if (param2[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,param2);
  } else {
    strcpy(tmp,param2);
  }
  complete_dir(param2,tmp);
  if (!sd.exists(params))
    error_code = 1;
  else if (sd.exists(param2))
    error_code = 4;
  else if (!sd.rename(params,param2)){
    sprintf_P(tmp,PSTR("can not rename file %s to %s"),params,param2);
    log_0(tmp);
    error_code = 3;
  }
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 8  OK
void cmd_copy(){
uint8_t error_code;
char s[MAX_FILENAME_LEN];
//char param1[MAX_FILENAME_LEN];
char param2[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
uint32_t result;
  set_SDLed(LED_ON);
  error_code = 0;

  ToggleClock();

  //GET PARAMS (first param)
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAM1=");Serial.println(params);


  ToggleClock();
  //GET PARAMS (second param)
  param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    param2[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  param2[param_len] = 0;
  // Serial.print("PARAM2=");Serial.println(param2);

//  split_two_params(params, param1, param2);
  // Serial.print("FILE1: ");Serial.println(params);
  // Serial.print("FILE2: ");Serial.println(param2);
  if (param2[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  complete_dir(params,tmp);
  
  if (param2[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,param2);
  } else {
    strcpy(tmp,param2);
  }
  complete_dir(param2,tmp);
  uint32_t i;
  boolean opened = Sfile.open(params,O_READ);
  i = Sfile.fileSize();
  // Serial.print("SIZE:");Serial.println(i);
  if (i>1024*200) {
    log_2(params);
//    send_str("file too large\r");
    error_code = 5;
  } else if (!opened) {
    log_2(params);
    error_code = 1;
//    send_str("can not open source file\r");
  } else if (!Dfile.open(param2,O_CREAT|O_WRITE|O_TRUNC)) {
    log_2(param2);
//    send_str("can not create destination file\r");
    error_code = 6;
  } else {
    i = 0;
    do {
      set_SDLed((millis() % 64)>32);
      result = Sfile.read(copy_buffer,BUFFSIZE);
      int error = Dfile.write(copy_buffer,result);
      if (error < 0) error_code = 7;
      i+=BUFFSIZE;
    } while (result==BUFFSIZE);
//    send_str("OK");
  }
  Sfile.close();
  Dfile.close();
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 9
void cmd_load(){
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
uint32_t result;
uint8_t error_code;
uint32_t i;
uint32_t fsize;
uint32_t fsize2;

  uint32_t st = millis();
  set_SDLed(LED_ON);

  error_code = 0;
  ToggleClock();

  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  char ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  if (param_len==0) {
    sprintf_P(params,PSTR("%s%03d"),params,file_counter);
    file_counter++;
  } else {
//    strcpy(last_file,params);
    file_counter = 0;
  }
  // Serial.print("PARAMS=");Serial.println(params);

  if (params[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  if (!sd.exists(tmp)) strcat(tmp,".p");
  // Serial.print("FILE: ");Serial.println(tmp);
  
  boolean opened = Sfile.open(tmp,O_READ);
  fsize = Sfile.fileSize();
  // Serial.print("SIZE: ");Serial.println(fsize);
  Sfile.seekSet(0);
  if (!opened || (fsize>65535) || (fsize == 0)) {
    log_0("can't open load file");
    // send file length = 0
    SendByteToZ80(0);
    SendByteToZ80(0);
    error_code = 1;
    if (fsize>65535) error_code = 5;
  } else {
    char* ext = get_filename_ext(tmp);
    upStr(ext);
    log_2("sending file");
;
    result = Sfile.read(copy_buffer,13);
    Sfile.seekSet(0);
//    fsize2 = fsize;
    if (strcmp(ext,"P")==0) 
      fsize2=copy_buffer[11]+256*copy_buffer[12]-16393;
    else fsize2 = fsize;
    // Serial.println(fsize2);
//  Serial.print("11: ");Serial.println(copy_buffer[11]);
//  Serial.print("12: ");Serial.println(copy_buffer[12]);
      
    // send file length
    SendByteToZ80(fsize2 & 0xff);
    SendByteToZ80((fsize2 >> 8) & 0xff);
    i = 0;
    do {
      set_SDLed((millis() % 64)>32);
      result = Sfile.read(copy_buffer,BUFFSIZE);
      for (uint16_t j=0; j<result; j++){
        SendByteToZ80(copy_buffer[j]); 
        i++;
        if (i>=fsize2) break;
      }
      // i+=result;
    } while ((i < fsize2) && (result==BUFFSIZE));
  }
  if (i<fsize2) {
    error_code = 8;
    for (; i<fsize2;i++){
      SendByteToZ80(0);
    }
  }
  Sfile.close();
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
  /// Serial.println(millis()-st);
}

// COMMAND = 10
void cmd_save(){
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
uint32_t result;
uint16_t block_size;
uint8_t error_code;

  set_SDLed(LED_ON);
  error_code = 0;
  ToggleClock();

  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  char ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  if (params[0]!='/'){
    complete_dir(tmp,current_dir);
    strcpy(tmp,current_dir);
    strcat(tmp,params);
  } else {
    strcpy(tmp,params);
  }
  // Serial.print("FILE: ");Serial.println(tmp);
  
  boolean opened = Sfile.open(tmp,O_CREAT|O_WRITE|O_TRUNC);

  // read file length
  ToggleClock();
  uint8_t l = GetByteFromZ80();
  ToggleClock();
  uint8_t h = GetByteFromZ80();
  uint16_t proglen = h*256+l;

  // Serial.print("SIZE:");Serial.println(proglen);
  Sfile.seekSet(0);
  if (!opened || (proglen == 0)) {
    log_0("can't save file");
    error_code = 1;
  };
  log_2("saving file");
//  delay(1000);
  uint32_t i = 0;
  do {
    if (proglen-i > BUFFSIZE) block_size = BUFFSIZE;
    else block_size = proglen-i;
    // Serial.print("Pos:"); Serial.print(i); Serial.print(" bytes=");Serial.println(block_size);
    set_SDLed((millis() % 64)>32);
    for (uint16_t j=0; j<block_size; j++){  
      ToggleClock();
      copy_buffer[j] = GetByteFromZ80();
//      Serial.print("j:"); Serial.print(j); Serial.print(" data=");Serial.println(data_in);
   }
    if (error_code == 0){
      result = Sfile.write(copy_buffer,block_size);
      if (result < block_size) error_code = 7;
    }
    i+=BUFFSIZE;
  } while (i<proglen);
  // Serial.println("FINISHING");
  Sfile.close();
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}


void cmd_dir_open(){
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
char dirname[MAX_FILENAME_LEN];
char wildcards[MAX_FILENAME_LEN];
int pos;
uint8_t error_code;
uint8_t c;
uint16_t nfiles;
uint16_t ndirs;
bool isdir = false;
  nfiles = 0;
  ndirs=0;
  error_code = 0;
  ToggleClock();
  
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  ToggleClock();
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
    ToggleClock();
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  complete_dir(s, params);
  if (dir.open(s))
    isdir = dir.isDir();
  else isdir = false;
  dir.close();
  if (isdir) {
     strcpy(dirname, s);
     strcpy(wildcards,"*");
  }
  else {
     sprintf_P(s, PSTR("%s%s"), current_dir, params);
    //Serial.print("1=");Serial.println(s);
     int len = strlen(s);
     // SPLIT INTO DIR AND FILENAME
    pos = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (s[i] == '/') {
            pos = i + 1;
            break;
        }
    }
    memcpy(wildcards, s + pos, len - pos);
    wildcards[len - pos] = 0;
    if (pos > 0) s[pos] = 0;
    complete_dir(dirname, s);
  }
  //Serial.print("DIR=");Serial.println(params);
  //Serial.print("FIL=");Serial.println(wildcards);

  set_SDLed(LED_ON);

  if (wildcards[0]==0) strcpy(wildcards,"*");
  if (!dir.open(dirname)){
    // Serial.println(dirname);
    log_0("dir open failed");
    error_code = 1;
  } else {
    // Serial.print("DIR=");Serial.println(dirname);
    sprintf_P(s,PSTR("dir of \"%s%s\" is \r"), dirname,wildcards);
    // Serial.println(s);
    
    // SEND STRING
    for (int i=0; s[i]!=0;i++){
      c = GetByteFromZ80();    // GET_CHAR
      if (c!=GET_NEXT_CHAR){
        command_active = c;
        return;
      }
      SendByteToZ80(pgm_read_byte(asc_to_asc81+s[i])); 
      ToggleClock();       // finish command
    }
    
    while (Sfile.openNext(&dir, O_RDONLY)) {
//      if ((millis() / 16)>8) troggle_SDLed();
      set_SDLed((millis() % 64)>32);
      Sfile.getName(file_name, MAX_FILENAME_LEN);
      if (Sfile.isDirectory() || matches(file_name,wildcards) || (wildcards[0]==0)){
        if (Sfile.isDir()) {
          strcat(file_name, "/");
          ndirs++;
          sprintf_P(s,PSTR("%s\r"),file_name);
        } else {
          nfiles++;
          uint32_t fsize = Sfile.fileSize();
          sprintf_P(s,PSTR("%-24s %7lu\r"),file_name, fsize);
        }
        //strcat(file_name,"\r");
        // SEND STRING
        for (int i=0; s[i]!=0;i++){
          c = GetByteFromZ80();    // GET_CHAR
          if (c!=GET_NEXT_CHAR){
            command_active = c;
            return;
          }
          SendByteToZ80(pgm_read_byte(asc_to_asc81+s[i])); 
          ToggleClock();       // finish command
        }
        
      }
      Sfile.close();
    }
  }  
  dir.close();
  sprintf_P(s,PSTR("Total %d dir(s), %d file(s) \r"), ndirs, nfiles);
  // SEND STRING
  for (int i=0; s[i]!=0;i++){
    c = GetByteFromZ80();    // GET_CHAR
    if (c!=GET_NEXT_CHAR){
      command_active = c;
      return;
    }
    SendByteToZ80(pgm_read_byte(asc_to_asc81+s[i])); 
    ToggleClock();       // finish command
  }

  c = GetByteFromZ80();    // GET_CHAR
  if (c!=GET_NEXT_CHAR){
    command_active = c;
    return;
  }
  SendByteToZ80(0xFF);        // EOT...
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  // Serial.print("DIR=");Serial.println(current_dir);
  reset_commands();
}

// COMMAND = GET_NEXT_CHAR
void cmd_get_next_char(){
  SendByteToZ80(0xFF);        // EOT...
  SendByteToZ80(0);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 14
void cmd_free_txt(){
char s[MAX_FILENAME_LEN];
uint32_t sdfree;
  set_SDLed(LED_ON);
  if (sdtotal == 0)
    sdtotal = sd.card()->sectorCount()>>1;
  sdfree = (sd.vol()->freeClusterCount() * sd.vol()->sectorsPerCluster())>>1;
  sprintf_P(s, PSTR("SD size %" PRIu32 " KB \rFREE %" PRIu32 " KB"), sdtotal, sdfree);
  // Serial.println(s);
  ToggleClock();
  // SEND STRING
  for (int i=0; s[i]!=0;i++){
    char c = GetByteFromZ80();    // GET_CHAR
    if (c!=GET_NEXT_CHAR){
      command_active = c;
      return;
    }
    SendByteToZ80(pgm_read_byte(asc_to_asc81+s[i])); 
    ToggleClock();       // finish command
  }
  char c = GetByteFromZ80();    // GET_CHAR
  if (c!=GET_NEXT_CHAR){
    command_active = c;
    return;
  }
  SendByteToZ80(0xFF);        // EOT...
  SendByteToZ80(0);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND =15
void cmd_free(){
uint32_t sdtotal;
uint32_t sdfree;
  set_SDLed(LED_ON);
  sdtotal = 0.000512*sd.card()->sectorCount();
  sdfree = sd.vol()->freeClusterCount() * sd.vol()->sectorsPerCluster() * 0.000512;
  SendByteToZ80(sdtotal & 0xff);
  SendByteToZ80(sdtotal >> 8 & 0xff);
  SendByteToZ80(sdtotal >> 16 & 0xff);
  SendByteToZ80(sdtotal >> 24 & 0xff);
  // Serial.print("SD SIZE: "); Serial.println(sdtotal);
  SendByteToZ80(sdfree & 0xff);
  SendByteToZ80(sdfree >> 8 & 0xff);
  SendByteToZ80(sdfree >> 16 & 0xff);
  SendByteToZ80(sdfree >> 24 & 0xff);
  SendByteToZ80(0);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
  // Serial.print("SD FREE: "); Serial.println(sdfree);
}

// COMMAND =16  
void cmd_opendir2(){
char s[MAX_FILENAME_LEN];
char file_name[MAX_FILENAME_LEN];
char wildcards[MAX_FILENAME_LEN];
int pos;
uint8_t error_code;
uint8_t c;
bool isdir = false;
uint16_t index_array;
  error_code = 0;
  ToggleClock();
  
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("DIR=");Serial.println(current_dir);
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  complete_dir(s, params);
  if (dir.open(s))
    isdir = dir.isDir();
  else isdir = false;
  dir.close();
  if (isdir) {
     strcpy(array_dirname, s);
     strcpy(wildcards,"*");
  }
  else {
     sprintf_P(s, PSTR("%s%s"), current_dir, params);
    // Serial.print("1=");Serial.println(s);
     int len = strlen(s);
     // SPLIT INTO DIR AND FILENAME
    pos = 0;
    for (int i = len - 1; i >= 0; i--) {
        if (s[i] == '/') {
            pos = i + 1;
            break;
        }
    }
    memcpy(wildcards, s + pos, len - pos);
    wildcards[len - pos] = 0;
    if (pos > 0) s[pos] = 0;
    complete_dir(array_dirname, s);
  }
  // Serial.print("DIR=");Serial.println(params);
  // Serial.print("FIL=");Serial.println(wildcards);

  set_SDLed(LED_ON);

  if (wildcards[0]==0) strcpy(wildcards,"*");
  if (!dir.open(array_dirname)){
    // Serial.println(array_dirname);
    log_0("dir open failed");
    error_code = 1;
  } else {
    // Serial.print("DIR=");Serial.println(array_dirname);
    sprintf_P(s,PSTR("dir of \"%s%s\" is \r"), array_dirname,wildcards);
    // Serial.println(s);

    index_array = 0;
    while (Sfile.openNext(&dir, O_RDONLY)) {
      set_SDLed((millis() % 64)>32);
      Sfile.getName(file_name, MAX_FILENAME_LEN);
      if ((Sfile.isDirectory() || matches(file_name,wildcards) || (wildcards[0]==0)) && (index_array < MAX_FILE_ARRAY)){
        file_array[index_array] = Sfile.dirIndex();
        // Serial.print(index_array); Serial.print(" - "); Serial.println(file_name); 
        index_array++;
      }
      Sfile.close();
    }
  }  
  dir.close();
//  SendByteToZ80((index_array >> 8)&0xff);   // number of files low part
//  SendByteToZ80(index_array&0xff);          // number of files hi part
  SendByteToZ80(error_code);                // ... and Status
  ToggleClock();                            // Final clock toggle
  // Serial.print("DIR=");Serial.println(current_dir);
  reset_commands();
}

// COMMAND =17
void cmd_getrowlen(){
  
  uint8_t error_code;
  uint8_t h,l;
  uint8_t len;
  char file_name[MAX_FILENAME_LEN];

  error_code = 0;
  ToggleClock();
  l = GetByteFromZ80();
  ToggleClock();
  h = GetByteFromZ80();
  row_array = h*256+l;
  if (!dir.open(array_dirname)) {
    file_name[0]=0;
    error_code = 1;
    len = 0;
  } else {
    Sfile.open(dir,row_array);
    Sfile.getName(file_name, MAX_FILENAME_LEN);
    len = strlen(file_name);
    Sfile.close();
  }
  dir.close();
  SendByteToZ80(len);
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}


// COMMAND =18
void cmd_getrow(){
  uint8_t h,l;
  
  uint8_t error_code;
  char file_name[MAX_FILENAME_LEN];
  uint8_t len;

  error_code = 0;
  ToggleClock();
  l = GetByteFromZ80();
  ToggleClock();
  h = GetByteFromZ80();
  row_array = h*256+l;
//  Serial.print("GETROW "); Serial.println(file_array[row_array]);
  if (row_array==0){
    strcpy(file_name,current_dir);
    error_code = 0;
    len = strlen(file_name);  
    SendByteToZ80(len);  // string len
    for (int i=0; i<len;i++){
      SendByteToZ80(pgm_read_byte(asc_to_asc81+file_name[i]));
    }
  } else if(!dir.open(array_dirname, O_RDONLY)) {
    // Serial.print("DIR NOT FOUND "); Serial.println(array_dirname);
    file_name[0]=0;
    error_code = 1;
    len = 0;
  } else {
    if (Sfile.open(&dir, file_array[row_array-1], O_RDONLY)){
      Sfile.getName(file_name, MAX_FILENAME_LEN);
      if (Sfile.isDir()){
        strcpy(tmp,file_name);
        sprintf(file_name,"<%s>",tmp);
      }
      len = strlen(file_name);
      SendByteToZ80(len);  // string len
      for (int i=0; i<len;i++){
        SendByteToZ80(pgm_read_byte(asc_to_asc81+file_name[i]));
      }
      Sfile.close();
    } else {
        SendByteToZ80(0);  // string len
        dir.getName(file_name, MAX_FILENAME_LEN);
        // Serial.print("DIR: "); Serial.println(file_name);
        // Serial.print("COULD NOT OPEN FILENO "); Serial.println(file_array[row_array]);
        error_code = 0;
    }
  }
  dir.close();
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 19
void cmd_enable_m1not() {
  log_2("M1NOT ENABLED");
  digitalWrite(EN_M1_NOT,HIGH);
  ToggleClock();
  
  reset_commands();
}

// COMMAND = 20
void cmd_disable_m1not() {
  log_2("M1NOT ENABLED");
  digitalWrite(EN_M1_NOT,LOW);
  ToggleClock();
  
  reset_commands();
}

// COMMAND = 22
void cmd_binary_say(){
  uint8_t error_code;
  uint8_t i;
  char dummy[3]={0,0,0};
  uint8_t value;
  uint8_t len;
  
  error_code = 0;
  ToggleClock();
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  len = param_len/2;
  for (i=0; i<len; i++){
    dummy[0]=params[i*2];
    dummy[1]=params[i*2+1];
    sscanf(dummy,"%x",&value); 
    v[i]=value;
    // Serial.print(v[i]); Serial.print(", ");
  }
  // Serial.println();
  v[i] = 0;
  v[i+1]=0;
  cur_alofone = 0;
  alofone_index = 0;
  alofone_write_pos = len+1;
  DAC_Init(DAC_VOICE);
  playAudio();
  DAC_Init(DAC_AY);
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 23
void cmd_say(){
  uint8_t error_code;
  uint8_t i;
  char dummy[3]={0,0,0};
  uint8_t value;
  uint8_t len;
  
  error_code = 0;
  ToggleClock();
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  len = param_len/2;
  for (i=0; i<len; i++){
    dummy[0]=params[i*2];
    dummy[1]=params[i*2+1];
    sscanf(dummy,"%x",&value); 
    v[i]=value;
    // Serial.print(v[i]); Serial.print(", ");
  }
  // Serial.println();
  v[i] = 0;
  v[i+1]=0;
  cur_alofone = 0;
  alofone_index = 0;
  alofone_write_pos = len+1;
  DAC_Init(DAC_VOICE);
  playAudio();
  DAC_Init(DAC_AY);
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 24
void cmd_AY_set_reg(){
  uint8_t reg;
  uint16_t value;
  uint8_t error_code;

  error_code = 0;
  ToggleClock();
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  reg = params[0];
  value = params[1];
  AY_reg[reg] = value;
  AY_run();
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 25
void cmd_AY_get_reg(){
  uint8_t reg;
  uint8_t value;
  uint8_t error_code;

  error_code = 0;
  ToggleClock();
  //GET PARAMS
  uint8_t param_len = GetByteFromZ80();
  // Serial.print("LEN=");Serial.println(param_len);
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (char) pgm_read_byte(asc81_to_ascii+ch);
  }
  params[param_len] = 0;
  // Serial.print("PARAMS=");Serial.println(params);

  SendByteToZ80(AY_reg[params[0]]);  // ... and Status
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 26
void cmd_AY_play(){
  uint8_t error_code;
  char param2[MAX_PARAM_LEN];
  char param3[MAX_PARAM_LEN];
  uint8_t param_len;

  error_code = 0;
  ToggleClock();
  //GET PARAM1
  param_len = GetByteFromZ80();
  byte ch;
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    params[i] = (unsigned char) pgm_read_byte(asc81_to_ascii+ch);
    if ((ch >= 0x26) && (ch <= 0x3f)) params[i] = tolower(params[i]);
  }
  params[param_len] = 0;
//  log_3("1 P1=%s P2=%s P3=%s", params, param2, param3);
  ToggleClock();
  //GET PARAM2
  param_len = GetByteFromZ80();
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    param2[i] = (unsigned char) pgm_read_byte(asc81_to_ascii+ch);
    if ((ch >= 0x26) && (ch <= 0x3f)) param2[i] = tolower(param2[i]);
  }
  param2[param_len] = 0;
  log_3("2 P1=%s P2=%s P3=%s", params, param2, param3);
  ToggleClock();
  //GET PARAM3
  param_len = GetByteFromZ80();
  log_3("3.0 len %d", param_len);
  for (uint8_t i=0; i<param_len; i++){
    ToggleClock();
    ch = GetByteFromZ80();
    param3[i] = (unsigned char) pgm_read_byte(asc81_to_ascii+ch);
    if ((ch >= 0x26) && (ch <= 0x3f)) param3[i] = tolower(param3[i]);
    log_3("3.0 len %c", param3[i]);
  }
  param3[param_len] = 0;
  log_3("3 P1=%s P2=%s P3=%s", params, param2, param3);
  DAC_Init(DAC_AY);
  error_code = Play_AY(params,param2,param3);
  log_3("error code:%d \n\r",error_code);
  
  SendByteToZ80(error_code);  // ... and Status
  ToggleClock();              // Final clock toggle
  reset_commands();
}

// COMMAND = 27
void cmd_sel_128_chars(){
//  SendByteToZ80(error_code);  // ... and Status
  log_2("128 chars mode selected");
  digitalWrite(SEL_128CHARS,HIGH);
  ToggleClock();
  
  reset_commands();
}

// COMMAND = 28
void cmd_sel_64_chars(){
  log_2("64 chars mode selected");
  digitalWrite(SEL_128CHARS,LOW);
  ToggleClock();
  reset_commands();
}

// COMMAND = 29
void cmd_fullpaging(){
  log_2("full paging mode selected");
  digitalWrite(FULLPAGING,HIGH);
  ToggleClock();
  reset_commands();
}

// COMMAND = 30
void cmd_halfpaging(){
  log_2("half paging mode selected");
  digitalWrite(FULLPAGING,LOW);
  ToggleClock();
  reset_commands();
}

// COMMAND = 32
void cmd_getbyte(){
  uint8_t value;
  ToggleClock();
  // Serial.println("GET BYTE");
  uint8_t reg = GetByteFromZ80();  
  // Serial.println(reg);
  
  if (reg <128) value = sys_vars[reg];
  else value = EEPROM.read(reg-128);
  
  SendByteToZ80(value);  
  // Serial.println(value);
  ToggleClock();
  reset_commands();
}

// COMMAND = 33
void cmd_setbyte(){
  // Serial.println("SET BYTE");
  ToggleClock();
  uint8_t reg = GetByteFromZ80();  
  // Serial.println(reg);
  ToggleClock();
  uint8_t value = GetByteFromZ80();  
  // Serial.println(value);
  ToggleClock();
  if (reg <128) sys_vars[reg]=value;  
  else EEPROM.write(reg-128,value);

  reset_commands();
}

// reserved codes for future
void cmd_spare(){
  Serial.print("Command not recognized: "); Serial.println(command_active);
  ToggleClock();
  SendByteToZ80(1);  // ... and Status
  reset_commands();
}

#define LAST_COMMAND 33
typedef void (*command_handler)(void);
command_handler commands[] = {
  cmd_nop,              //0
  cmd_ver,              //1
  cmd_pwd,              //2
  cmd_cd,               //3
  cmd_del,              //4
  cmd_mkdir,            //5
  cmd_rmdir,            //6
  cmd_move,             //7
  cmd_copy,             //8
  cmd_load,             //9
  cmd_save,             //10
  cmd_spare,            //11
  cmd_dir_open,         //12
  cmd_get_next_char,    //13
  cmd_free_txt,         //14
  cmd_free,             //15
  cmd_opendir2,         //16
  cmd_getrowlen,        //17
  cmd_getrow,           //18
  cmd_enable_m1not,     //19
  cmd_disable_m1not,    //20
  cmd_spare,            //21
  cmd_binary_say,       //22
  cmd_say,              //23
  cmd_AY_set_reg,       //24   
  cmd_AY_get_reg,       //25
  cmd_AY_play,          //26
  cmd_sel_128_chars,    //27
  cmd_sel_64_chars,     //28
  cmd_fullpaging,       //29
  cmd_halfpaging,       //30
  cmd_spare,            //31
  cmd_getbyte,          //32
  cmd_setbyte           //33
};
