void jmpfar();

////#define  TRACE_CLOCK(s) Serial.print(s); Serial.println(clock);
//#define  TRACE_CLOCK(s)
//
void inline idle_tasks(){
  if (playing_wav) {
    load_wav_buffer();
    if (playing_pos >= wavsize){
      playing_wav = false;
      wav_file.close();
      DAC_Init(DAC_AY);
      wav_file.close();
    }
  } else {
    if (playing_VGM) {
      if ((millis() >= ti_VGM))  {
        troggle_SDLed();
        read_next_VGM_command();
      } // else waiting.......
    }
  
    if (playing_PEG) {
      exec_PEG();
    }
  }
}


inline void set_clock(bool c){
  clock = c;
  digitalWrite(CTRL_CLK, clock);
}

//inline void change_clock(){
//  set_clock(!clock);
////  Serial.print("CLOCK: "); Serial.print(clock); Serial.print(" N="); Serial.println(clk_count++);
//}
  
inline void reset_commands(){
  command_active = CMD_IDLE;
  data_ready = false;
}

inline void ToggleClock()
{
  set_clock(!clock);
}

uint8_t GetByteFromZ80()
{
  do /*nothing*/; while (!digitalRead(GET_DATA_REG));
  digitalWrite(RST_DATA_REG, LOW);
  GPIOR1 = 0;
  digitalWrite(RST_DATA_REG, HIGH);
  return PINF;
}

void SendByteToZ80(uint8_t byte)
{
  PORTK = byte;
  ToggleClock();
  do /*nothing*/; while (!digitalRead(GET_DATA_REG));
  digitalWrite(RST_DATA_REG, LOW);
  GPIOR1 = 0;
  digitalWrite(RST_DATA_REG, HIGH);
}

uint8_t GetByteFromZ80_IT()
{
  do /*nothing*/idle_tasks(); while (!digitalRead(GET_DATA_REG));
  digitalWrite(RST_DATA_REG, LOW);
  GPIOR1 = 0;
  digitalWrite(RST_DATA_REG, HIGH);
  return PINF;
}

void SendByteToZ80_IT(uint8_t byte)
{
  PORTK = byte;
  ToggleClock();
  do /*nothing*/idle_tasks(); while (!digitalRead(GET_DATA_REG));
  digitalWrite(RST_DATA_REG, LOW);
  GPIOR1 = 0;
  digitalWrite(RST_DATA_REG, HIGH);
}


inline void _rst_ctrl_reg(uint8_t status) {
  rst_ctrl_reg=status;
  digitalWrite(RST_CTRL_REG, status);
}
  
inline void _rst_data_reg(uint8_t status) {
  rst_data_reg=status;
  digitalWrite(RST_DATA_REG, status);
}

void get_ctrl_reg(void){
   _rst_ctrl_reg(LOW); 
  reset_commands();
  reseted = true;
  initialised = false;
//  interrupts();
  Serial.println("RESET");
  delay(10);
  _rst_ctrl_reg(HIGH);
  ToggleClock();
  GPIOR0 |= 2; // software reset
  __asm("sei\n%~jmp %x0" :: "i" (jmpfar));
}

void get_data_reg(void){
  data_in = PINF;
  _rst_data_reg(LOW);
//  Serial.print("DATAIN=");Serial.println(data_in);
  data_ready = true;
 _rst_data_reg(HIGH);
  if (command_active==CMD_IDLE){
    command_active = data_in;
  }
}

inline void read_data(){
//  if (digitalRead(GET_CTRL_REG)) {get_ctrl_reg();}
  if (digitalRead(GET_DATA_REG)) {get_data_reg();}
}
