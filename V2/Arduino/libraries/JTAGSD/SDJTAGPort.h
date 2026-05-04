#ifndef JTAGPORT_H
#define JTAGPORT_H

/*
 * Making this class virtual makes the code bigger (18 bytes) and slower. But
 * it is the right thing to do if we want to reuse the code in a non-messy way.
 */
#include <inttypes.h>

class ArduinoPin
{
private:
  uint8_t m_bit;
  uint8_t m_port;

  void setBit(uint8_t n) { m_bit = n; }
  void setPort(uint8_t n) { m_port = n; }

  uint8_t m_pin;
  uint8_t m_mode;

protected:
  uint8_t getBit() const { return m_bit; }
  uint8_t port() const { return m_port; }

public:
  ArduinoPin(uint8_t pin, uint8_t mode);
  ~ArduinoPin() {}

  uint8_t pin() const { return m_pin; }

  uint8_t mode() const { return m_mode; }
};

#ifdef ARDUINO_ARCH_AVR
  #define GPIO_REGISTER_TYPE uint8_t
#elif defined ARDUINO_ARCH_ESP8266
  #define GPIO_REGISTER_TYPE uint32_t
#elif defined ARDUINO_ARCH_ESP32
  #define GPIO_REGISTER_TYPE uint32_t
#else
  #define GPIO_REGISTER_TYPE uint8_t
#endif

class ArduinoInputPin : public ArduinoPin
{
private:
  volatile GPIO_REGISTER_TYPE *m_in;

  volatile GPIO_REGISTER_TYPE *in() const { return m_in; }

public:
  ArduinoInputPin(uint8_t pin);
  ~ArduinoInputPin() {}

  uint8_t read() const { return *in() & getBit(); }
};


class ArduinoOutputPin : public ArduinoPin
{
private:
  volatile GPIO_REGISTER_TYPE *m_out;

  volatile GPIO_REGISTER_TYPE *out() const { return m_out; }

public:
  ArduinoOutputPin(uint8_t pin);
  ~ArduinoOutputPin() {}

  void clr() const { *out() &= ~getBit(); }
  void set() const { *out() |= getBit(); }
};

class JTAGPort
{
public:
	JTAGPort() {}
	virtual ~JTAGPort() {}

	virtual void pulse_clock() = 0;
	virtual bool pulse_clock_and_read_tdo() = 0;
	virtual void set_tms() = 0;
	virtual void clr_tms() = 0;
	virtual void set_tdi() = 0;
	virtual void clr_tdi() = 0;
	virtual bool read_vref() const = 0;
};

#endif  // JTAGPORT_H
