
#define DAC_BUFFER_SIZE 512
#define DAC_AY    0
#define DAC_VOICE 1

#define DAC_out DAC1->DHR12R1

typedef void (*t_DAC_handler)(void);

t_DAC_handler DAC_handler;

const int SPEAKER_PIN = 45;
volatile uint16_t DAC_work_buffer_count = 0;
volatile uint16_t DAC_live_buffer_count = 0;
volatile uint16_t DAC_live_buffer_pos = 0;
volatile uint8_t DAC_buffer1[DAC_BUFFER_SIZE+20];
volatile uint8_t DAC_buffer2[DAC_BUFFER_SIZE+20];
volatile bool DAC_live_buffer_empty = true;
volatile uint8_t *DAC_work_buffer;
volatile uint8_t *DAC_live_buffer;
volatile uint8_t *DAC_dummy;
volatile uint8_t ocr = 0;
volatile uint8_t DAC_AY_data[3] = {0,0,0};
volatile bool DAC_AY_active = false;
volatile uint8_t fact = 1;
volatile uint16_t nextOCR = 0;
volatile uint16_t VOC_data = 0;
volatile uint16_t VOC_counter = 0;

struct {
  bool signal;
  uint16_t counter;
  uint16_t noise_period;
  uint16_t noise_enabled;
  uint16_t noise_level;
  uint16_t noise_counter;
  uint32_t rng;
  uint16_t tone_period;
  uint16_t tone_enabled;
  uint16_t tone_level;
  uint16_t tone_freq;
  uint16_t max_count;
//  uint16_t tone_tics;
  uint16_t volume; 
 uint16_t env_enabled;
} AY_config[4];

struct {
  boolean enabled;
  uint16_t period;
  uint32_t pulses5;
  uint16_t phase;
  uint16_t increment;  
  uint16_t inc_counter;  
  uint16_t level;
  // uint16_t counter;
  uint16_t typ;
  void (*handle)();
} AY_env;

void AY_synthetize(void);

void AY_handler();
void VOICE_handler();

void DAC_Init(uint8_t DAC_type){
  noInterrupts();
  initialised = true;
  if (DAC_type == DAC_AY) {
    DAC_handler = AY_handler;
    DAC_work_buffer = DAC_buffer1;
    DAC_live_buffer = DAC_buffer2;
    DAC_AY_active = false;
    DAC_freq = 22000;
  } else {
    DAC_handler = VOICE_handler;
    DAC_work_buffer = DAC_buffer1;
    DAC_live_buffer = DAC_buffer2;
    DAC_AY_active = false;
    DAC_freq = 11000;
  }
  
  interrupts();
  TIM_TypeDef *Instance = TIM2;
  HardwareTimer *MyTim = new HardwareTimer(Instance); 
  MyTim->setOverflow(DAC_freq*2, HERTZ_FORMAT);
  MyTim->attachInterrupt(DAC_handler);
  analogWrite(PA4,4095);
  MyTim->resume();
  
}


bool DAC_buffer_full(void){
  return DAC_work_buffer_count >= DAC_BUFFER_SIZE;
}

void DAC_add_sample(uint8_t sample){
  if (!DAC_buffer_full())
    DAC_work_buffer[DAC_work_buffer_count++]=sample;
  DAC_live_buffer_empty = false;
}

void DAC_add_buffer(char* buffer, uint16_t size){
  if (DAC_work_buffer_count + size >= DAC_BUFFER_SIZE){
    memcpy((void*) DAC_work_buffer+DAC_work_buffer_count, buffer, DAC_BUFFER_SIZE-DAC_work_buffer_count);
    DAC_work_buffer_count = DAC_BUFFER_SIZE;
  } else {
    memcpy((void*) DAC_work_buffer+DAC_work_buffer_count, buffer, size);
    DAC_work_buffer_count += size;
  }
  DAC_live_buffer_empty = false;
}

void DAC_feed(void){
  while (DAC_live_buffer_pos < DAC_live_buffer_count) {}
  DAC_dummy = DAC_work_buffer;
  DAC_work_buffer = DAC_live_buffer;
  DAC_live_buffer = DAC_dummy;
  DAC_live_buffer_count = DAC_work_buffer_count;
  DAC_live_buffer_pos = 0;
  DAC_work_buffer_count = 0;
  DAC_live_buffer_empty = false;
}

void AY_handler(){
  AY_synthetize();
  DAC_out = (( DAC_AY_data[0]+DAC_AY_data[1]+DAC_AY_data[2])) >> 2;  // AY 
}

void VOICE_handler(){
  if (DAC_live_buffer_pos < DAC_live_buffer_count) {
    VOC_data = DAC_live_buffer[DAC_live_buffer_pos++];
  }
  DAC_out = VOC_data*4;  // voice 
}
