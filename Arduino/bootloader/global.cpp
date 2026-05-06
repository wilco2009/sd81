#include "global.h"
#include "SD.h"

char logfile_name[] = "firmware.log";
File log_file;

void log(char* text){
  char s[100];
  log_file = SD.open(logfile_name, FILE_WRITE);
  sprintf(s,"firmware %s", text);
  log_file.println(s);
  log_file.close();
}