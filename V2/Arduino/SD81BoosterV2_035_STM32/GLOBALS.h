//#define reversed
 #define LOG0
 #define LOG1
 #define LOG2
// #define LOG3

#ifndef SERIAL_RX_BUFFER_SIZE
#define SERIAL_RX_BUFFER_SIZE 64
#endif /* SERIAL_RX_BUFFER_SIZE */

// #define COMMON_ANODE

#define clRED     0xff0000
#define clGREEN   0x00ff00
#define clBLUE    0x0000ff
#define clCYAN    0x00ffff
#define clMAGENTA 0xff00ff
#define clYELLOW  0xffff00
#define clWHITE   0xffffff
#define clORANGE  0xffa500
#define clPINK    0xf08080

#define CHECK_ROM

#define VERSION 0x10
#define CPLD_NAME "/SD81CPLD"
#define MAX_FILENAME_LEN 100
#define MAX_PARAM_LEN 100

#define BUFFSIZE 512
#define MAX_FILE_ARRAY BUFFSIZE/2

#define GET_NEXT_CHAR 13

#define CMD_NOP       0
#define CMD_VER       1
#define CMD_DIR       2
#define CMD_CD        3
#define CMD_IDLE    255

#define END_BYTE    255

#define LED_OFF HIGH
#define LED_ON  LOW

#define SERIAL_SPEED 9600

#define lapse for(int i=0;i<10;i++)

#define REVERSE_BITS8(x) ( \
    (((x) & 0x80) >> 7) | \
    (((x) & 0x40) >> 5) | \
    (((x) & 0x20) >> 3) | \
    (((x) & 0x10) >> 1) | \
    (((x) & 0x08) << 1) | \
    (((x) & 0x04) << 3) | \
    (((x) & 0x02) << 5) | \
    (((x) & 0x01) << 7) )

//XModem xmodem;

uint32_t DAC_freq;

uint8_t GPIOR0 = 0; // Esta variable tiene que ser sustituida por alguna forma de preservar el valor después dee un reset

uint8_t copy_buffer[BUFFSIZE];
char array_dirname[MAX_FILENAME_LEN];
uint16_t* file_array; //[MAX_FILE_ARRAY];
uint8_t sys_vars[128];
uint16_t row_array;
uint32_t serial_ini;
boolean serial_end;

boolean T81_dir = false;

uint32_t sdtotal = 0;
bool sdled = false;
uint32_t clk_count = 0;
volatile bool comm_clk = 0;
volatile uint8_t command_active = CMD_IDLE;
uint8_t param_len;
char params[MAX_PARAM_LEN] = "";
char tmp[MAX_FILENAME_LEN]="";

//char last_file[MAX_FILENAME_LEN]="";
uint8_t file_counter = 0;

volatile uint8_t data_in = CMD_IDLE;
volatile bool data_ready = false;

volatile bool rst_ctrl_reg = HIGH;
volatile bool rst_data_reg = HIGH;
bool reseted = false;
bool initialised = false;

SdFile dir;
SdFile Sfile;
SdFile Dfile;
SdFile TmpFile;

const char asc_to_asc81 [] PROGMEM = {
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x77,0x0f,0x76,0x0f,0x0f,0x76,0x0f,0x0f, //00..0f
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //10..1f
  0x00,0x0f,0x0b,0x0f,0x0d,0x0f,0x0f,0x0f,0x10,0x11,0x17,0x15,0x1a,0x16,0x1b,0x18, //20..2f
  0x1c,0x1d,0x1e,0x1f,0x20,0x21,0x22,0x23,0x24,0x25,0x0e,0x19,0x13,0x14,0x12,0x0f, //30..3f
  0x0f,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34, //40..4f
  0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x0f,0x0f,0x0f,0x0f,0x0f, //50..5f
  0x0f,0x26,0x27,0x28,0x29,0x2a,0x2b,0x2c,0x2d,0x2e,0x2f,0x30,0x31,0x32,0x33,0x34, //60..6f
  0x35,0x36,0x37,0x38,0x39,0x3a,0x3b,0x3c,0x3d,0x3e,0x3f,0x0f,0x0f,0x0f,0x0f,0x0f, //70..7f
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //80..8f
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //90..9f
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //a0..af
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //b0..bf
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //c0..cf
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //d0..df
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //e0..ef
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0xff  //f0..ff
  };

  const char asc81_to_ascii [] PROGMEM = {
   ' ', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '"', '&', '$', ':', '?', //00..0f
   '(', ')', '>', '<', '=', '+', '-', '*', '/', ';', ',', '.', '0', '1', '2', '3', //10..1f
   '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', //20..2f
   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', //30..3f
   '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', //40..4f
   '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', //50..5f
   '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', //60..6f
   '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', //70..7f
   ' ', '?', '?', '?', '?', '?', '?', '?', '?', '?', '?', '"', '&', '$', ':', '?', //80..8f
   '(', ')', '>', '<', '=', '+', '-', '*', '/', ';', ',', '.', '0', '1', '2', '3', //90..9f
   '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', //a0..af
   'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', //b0..bf
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //c0..cf
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,' ', //d0..df
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f, //e0..ef
  0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f,0x0f  //f0..ff
  }; 

inline uint8_t hexval(char x) { return x <= '9' ? x - '0' : (x & 0xDF) - 'A' + 10; }

inline void set_SDLed(bool state){
  sdled = state;
  digitalWrite(SD_LED, sdled);
}
inline void troggle_SDLed(){
  set_SDLed(!sdled);
}

void set_status_LED(uint8_t R,uint8_t G, uint8_t B){
#ifdef COMMON_ANODE
  analogWrite(ST_LED_R, (0xff-R)*1/1); 
  analogWrite(ST_LED_G, (0xff-G)*1/10); 
  analogWrite(ST_LED_B, (0xff-B)*1/2); 
#else
  analogWrite(ST_LED_R, R*1/1); 
  analogWrite(ST_LED_G, G*1/4); 
  analogWrite(ST_LED_B, B*1/8); 
#endif
}

void set_status_LED(uint32_t RGB){
//  set_status_LED((RGB >> 16)&0xff, (RGB >> 8)&0xff, RGB & 0xff);
  set_status_LED((RGB >> 16)&0xff, (RGB >> 8)&0xff, RGB & 0xff);
}

void LED_error(uint32_t RGB1, uint32_t RGB2){
    while (1) {
      set_status_LED(RGB1);
      delay(500);
      set_status_LED(RGB2);  
      delay(500);
     }
}
void LED_error(uint32_t RGB){
  LED_error(RGB, clRED);
}

char current_dir[MAX_FILENAME_LEN] = "/";
char S[100];

void split_two_params(char* params, char* param1, char* param2){
  int i,j;
  for (i=0; params[i]!=' '; i++){
    if (params[i]!=0)
      param1[i]=params[i];
    else break;
  }
  param1[i]=0;
  if (params[i]!=0){
    i++;
    for (j=i;(params[j]!=0); j++){
      param2[j-i]=params[j];
    }
    param2[j-i]=0;
  } else param2[0]=0;
}

void hex_to_str(char* v, char* params){
  uint8_t i;
  uint8_t len = strlen(params);
  for (i=0; i<len/2;i++){
    sscanf(params, "%2hhx", v+i);
    params+=2;
  }
  v[i]=0;
}

void upStr(char* s){
  for (int i=0; s[i]!=0; i++){
    s[i] = toupper(s[i]);
  }
}

#if defined(LOG0)||defined(LOG1)||defined(LOG2)||defined(LOG3)
void log_debug(const char* fmt PROGMEM, ...)
{
char S[60];
    va_list arg;
    va_start(arg, fmt);

    vsprintf_P(S,fmt, arg); 
    //vprintf can be replaced with vsprintf (for sprintf behavior) 
    //or any other printf function preceded by a v
    Serial.println(S);

    va_end(arg);
}
#endif

#ifdef LOG0
#define log_0(x, ...) log_debug(PSTR(x),##__VA_ARGS__)
#else
#define log_0(...)
#endif

#ifdef LOG1
#define log_1(x, ...) log_debug(PSTR(x),##__VA_ARGS__)
#else
#define log_1(...)
#endif

#ifdef LOG2
#define log_2(x, ...) log_debug(PSTR(x),##__VA_ARGS__)
#else
#define log_2(...)
#endif

#ifdef LOG3
#define log_3(x, ...) log_debug(PSTR(x), ##__VA_ARGS__)
#else
#define log_3(...)
#endif


void start_reset_Z80(){
  digitalWrite(Z80_RESET, LOW);
  pinMode(Z80_RESET, OUTPUT);
  digitalWrite(Z80_RESET, LOW);
}

void end_reset_Z80(){
  pinMode(Z80_RESET, INPUT);
  digitalWrite(Z80_RESET, HIGH);
}
