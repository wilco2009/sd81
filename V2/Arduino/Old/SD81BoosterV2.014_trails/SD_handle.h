
#define _DIR_START    0
#define _DIR_NAME     1
#define _FIRST_DOT    2
#define _SECOND_DOT   3
#define _REMOVE_DIRS  4

#define _DIR_UNDER_ROOT   -1
#define _NOT_A_DIR        -2
#define _NOT_EXISTS       -3

bool SDOK = false;
SdFat sd;

void SD_Init(void){
  if (!SDOK) {
    SDOK = sd.begin(SS_SD);   
    if (!SDOK) {
      log_0("SD initialization failed!",0);
      return;
    } else
      log_0("SD initialization done.",0);
  } else log_0("SD already initialized.",0);
}
boolean absolute_dir(char* fname){
  return fname[0]=='/';
}
boolean ext_present(char* fname){
  int i;
  for (i=strlen(fname)-1; i>=0; i--)
    if (fname[i]=='/') return false;
    else if (fname[i]=='.') return true;
  return false;
}

int remove_dotdirs(char* dest){
  int i,i2;
  byte c=0;
  char tmp[MAX_FILENAME_LEN];
  int len;
  int nd = 0; 

//  serial_printf("1. dest=%s \n\r", dest);
  len = strlen(dest);
  i2 = 0;
  for(i=len-1; i>=0;i--){
//    serial_printf("%d(%c) ",c,dest[i]);
    switch (c) {
      case _DIR_START:
        if (dest[i]=='/') {tmp[i2]=dest[i];i2++;}
        else if (dest[i]!='.') {tmp[i2]=dest[i];i2++; c = _DIR_NAME;}
        else c = _FIRST_DOT;
        break;
      case _DIR_NAME:
        tmp[i2]=dest[i];i2++; 
        if (dest[i]=='/') c = _DIR_START;
        break;
      case _FIRST_DOT: // first dot
        if (dest[i]=='/') {
          if (nd > 0) c = _REMOVE_DIRS;
          else c=_DIR_START;
        }
        else if (dest[i]!='.') {
          if (nd > 0) c = _REMOVE_DIRS;
          else {tmp[i2]='.';i2++;tmp[i2]=dest[i];i2++;c=_DIR_NAME;}
        } else c=_SECOND_DOT;
        break;
      case _SECOND_DOT: // second dot
        if (dest[i]!='/') {tmp[i2]='.';i2++;tmp[i2]='.';i2++;tmp[i2]=dest[i];i2++;c=_DIR_NAME;}
        else {c=_REMOVE_DIRS;nd++;}
        break;
      case _REMOVE_DIRS: // removing previous dirs (nd = number of folders to be removed)
        if (dest[i]=='/') {nd--; if (nd==0) c = _DIR_START;}
        else if (dest[i]=='.') {c = _FIRST_DOT;}
        break;
    }
  }
  if (nd>0) return _DIR_UNDER_ROOT;
  tmp[i2]='\0';
//  serial_printf("2. dest=%s tmp=%s \n\r", dest, tmp);

  len = strlen(tmp);
  for (i=0; i<len;i++){
    dest[i]=tmp[len-i-1];
  }
  dest[i]='\0';

//  serial_printf("3. dest=%s tmp=%s \n\r", dest, tmp);
  return 1;
  
}

int complete_dir(char* dest,char* source){
  int n;
    if (!absolute_dir(source)){
      sprintf_P(dest,PSTR("%s%s"),current_dir,source);
    } else sprintf_P(dest,PSTR("%s"),source);

    int err = remove_dotdirs(dest);
    if (err < 0) return err;

    n = strlen(dest)-1;
    if (dest[n]!='/') {sprintf_P(dest,PSTR("%s/"),dest);}
  
    if (TmpFile.open(dest)){
      if (TmpFile.isDirectory()){
        TmpFile.close();
        return 1;
      } else {
        TmpFile.close();
        return _NOT_A_DIR;
      } 
    } else return _NOT_EXISTS;
}

char* getfname(char* filename){
  char* str = strrchr(filename, '/');
  if (str==0) return filename;
  else return ++str;
}

void split_fname(char* source, char* dir, char* fname){
  int i;
  int len = strlen(source);
  for (i=len-1; (source[i]!='/')&&(i>=0); i--);
  memcpy(fname,source+i+1,len-i-1);
  fname[len-i-1]='\0';
  memcpy(dir,source,i+1);
  dir[i+1] = '\0';
}

const char *get_filename_ext(const char *filename) {
    const char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

void complete_fname(char* dest,char* source){
  char ext[]=".BAS";
  char dir[MAX_FILENAME_LEN];
  char fname[MAX_FILENAME_LEN];
  if (ext_present(source)) ext[0]='\0';
//  if (!absolute_dir(source)){
//    sprintf(dest,"%s%s%s",current_dir,source,ext);
//  } else 
  sprintf_P(dest,PSTR("%s%s"),source,ext);
  split_fname(dest,dir,fname);
  complete_dir(dest,dir);
  sprintf_P(dest,PSTR("%s%s"),dest,fname);
}

int load_ROM(char* rom_file){
  SdFile f;
  long startTime, stopTime;
  log_2("Reading ROM file \"%s\"",rom_file);
  startTime = millis();
  if (f.open(rom_file)){
    f.read(extmem,f.fileSize());
//    f.seekSet(0);
//    f.read(extmem+8192,f.fileSize());
#ifdef CHECK_ROM
    f.seekSet(0);
    for (int i=0; i< f.fileSize(); i++){
      uint8_t b = f.read();
      if (extmem[i] != b) {
        log_0("Error in mem pos %d",i);
        return -1;
      }
    }
#endif
    stopTime = millis();
    log_2("ROM file \"%s\" read to memory in %d ms",rom_file,stopTime-startTime);
    f.close();
  } else {
    log_0("Error openning ROM file",0);
    return -2;
  }
  return 0;
}
