static volatile uint8_t *extmem = reinterpret_cast<volatile uint8_t*>(0x8000);

static void mem_testOk() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);
  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}

static void mem_testFail() {
  for (uint8_t i=0; i<3; ++i) {
    digitalWrite(LED_BUILTIN, HIGH);
    delay(50);
    digitalWrite(LED_BUILTIN, LOW);
    delay(50);
  }
}

void enableMem(){
  log_2("Enabling external memory interface");
  bitSet(XMCRA, SRE);  // enable externalmemory
  bitSet(XMCRB, XMM0); // release unused pin PC7

  // SRW11=0 SW10=0 (Wait cycles: 0)
  // SRW11=0 SW10=1 (Wait cycles: 1)
  // SRW11=1 SW10=0 (Wait cycles: 2)
  // SRW11=1 SW10=1 (Wait cycles: 2 + 1)

  log_2("1. Wait cycles: 0");
  bitClear(XMCRA, SRW11);
  bitClear(XMCRA, SRW10);

  pinMode(A15b,OUTPUT); 
  pinMode(A16b,OUTPUT); 
  pinMode(A17b,OUTPUT); 
  pinMode(A18b,OUTPUT); 
  pinMode(nMEM_CE,OUTPUT); 
  digitalWrite(A15b,LOW);
  digitalWrite(A16b,LOW);
  digitalWrite(A17b,LOW);
  digitalWrite(A18b,LOW);
  digitalWrite(nMEM_CE,LOW);
  digitalWrite(nMEM_LATCH_OE,LOW);

}

void disableMem(){
  log_2("Disabling external memory interface");
  bitClear(XMCRA, SRE);  // disable externalmemory
  pinMode(A15b,INPUT); 
  pinMode(A16b,INPUT); 
  pinMode(A17b,INPUT); 
  pinMode(A18b,INPUT); 
  pinMode(nMEM_CE,INPUT); 
  pinMode(nMEM_OE,INPUT); 
  digitalWrite(nMEM_LATCH_OE,HIGH);
  DDRA = 0b00000000;
  DDRC = 0b00000000;
  pinMode(D0, INPUT_PULLUP);
  pinMode(D1, INPUT_PULLUP);
  pinMode(D2, INPUT_PULLUP);
  pinMode(D3, INPUT_PULLUP);
  pinMode(D4, INPUT_PULLUP);
  pinMode(D5, INPUT_PULLUP);
  pinMode(D6, INPUT_PULLUP);
  pinMode(D7, INPUT_PULLUP);
}

bool mem_test() {

  char s[100];

  log_2("WRITTING");
  srand(1);
  for (uint16_t i = 0; i<32768; i++) {
    byte v = rand()&0xFF;
      extmem[i] = v;
  }

  log_2("READING");
  srand(1);
  for (uint16_t i = 0; i<32768; i++) {
    byte dummy = extmem[i];
    byte v =rand()&0xFF;
    if (dummy !=v)
    {
      log_0("BAD! ADDR=%d READ=%d WRITE=%d",0, i, dummy, v);
      return false;
    } 
  }
  log_2("DONE");
  return true;
}


auto do_mem_Test = [&]() {
  long startTime, stopTime;
  bool res;

  startTime = millis();
  res = mem_test();
  stopTime = millis();
  if (res) {
    //log_1("MEM TEST PASSED, time (ms): %d",stopTime-startTime);
    mem_testOk();
  } else {
    //log_0("MEM TEST FAILED");
    mem_testFail();
  }
};
