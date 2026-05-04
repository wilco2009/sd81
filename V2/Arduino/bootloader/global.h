#ifndef GLOBAL_H
#define GLOBAL_H

#include <SD.h>

//#define DEBUG
#define LED_OFF HIGH
#define LED_ON  LOW
#define clRED     0xff0000
#define clGREEN   0x00ff00
#define clBLUE    0x0000ff
#define clCYAN    0x00ffff
#define clMAGENTA 0xff00ff
#define clYELLOW  0xffff00
#define clWHITE   0xffffff
#define clORANGE  0xffa500
#define clPINK    0xf08080
#define clBLACK   0x000000

extern char logfile_name[];
extern File log_file;
void log(char* text);

#endif
