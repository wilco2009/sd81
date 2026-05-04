#define ioLatch        GPIOD
#define VBAT_IN        PC1    // only as reference. see VBAT.cpp
#define ST_LED_R       PC8
#define ST_LED_B       PC7
#define ST_LED_G       PC6
#define DEBUG_RDY      PC13
#define FPGA_RESET      PB9
//#define nMEM_LATCH_OE  7
//#define TMS            8
//#define TDI            9
//#define TDO           10
//#define TCK           11
//#define VREF          12
#define RST_CTRL_REG  PB14
#define CTRL_CLK      PB4
#define RST_DATA_REG  PB5
#define GET_CTRL_REG  PC9
#define GET_DATA_REG  PC10
#define nOE_OL        PB10
#define A16b          PB11 
#define A17b          PB12 
#define A18b          PB13 
#define A15b          PE15
#define A14b          PE14
#define A13b          PE13
#define A12b          PE12
#define A11b          PE11
#define A10b          PE10
#define A9b           PE9
#define A8b           PE8
#define A7b           PE7
#define A6b           PE6
#define A5b           PE5
#define A4b           PE4
#define A3b           PE3
#define A2b           PE2
#define A1b           PE1
#define A0b           PE0
//#define nMEM_CE       38
//#define ALE           39
//#define nMEM_OE       40
#define nWAIT         PA0
//#define FULLPAGING    PA1
#define DEBUG_CLOCK   PA1
#define EN_M1_NOT     PA2
#define nWRb          PA3

#define SD_LED        PC0
//#define INT_CHR_TABLE 48
#define SEL_128CHARS  PA15
#define MISO          PA6
#define MOSI          PA7
#define SCLK          PA5
#define SS_SD         PB15  

#define SDA           PB7
#define SCL           PB6

#define CFG_DATA      PB3
#define CFG_RESET     PB1
#define CFG_CLK       PB0

// NOT USED?
#define ROM_WR_EN     PA8 
#define PS2_DATA      PC11
#define PS2_CLOCK     PC12

// CAMBIAR CON NUEVA VERSION
#define WAIT_SERIAL  PA10  // cambiar?
// #define FPGA_M0      PC12  // poner a 1 en el reset  (A4) PS2DATA NARANJA
// #define FPGA_M1      PC11  // poner a 0 en el reset  (A8) PS2CLOCK ROJO
#define Z80_RESET   PC11
#define FPGA_HSWAPEN PB5

// #define OUTPUT_LATCH  PORTD         
// #define INPUT_LATCH   PIND >> 8

// SINONIMOS
#define nMEM_OEb       PA8
#define Data           GPIOD
#define Addr           GPIOE

#define QSPIN          PC12 // Buscar pin libre


//#define D0            22
//#define D1            23
//#define D2            24
//#define D3            25
//#define D4            26
//#define D5            27
//#define D6            28
//#define D7            29
