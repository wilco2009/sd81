#ifdef ARDUINO_ARCH_AVR

#ifndef PLAYXSVFJTAGAVR_H
#define PLAYXSVFJTAGAVR_H

#include "SDJTAGPortAVR.h"
#include "SDPlayXSVF.h"
#include "SDSerialComm.h"
#include "SDXSVFPlayerJTAG.h"

class PlayXSVFJTAGAVR : public PlayXSVF
{
private:
	SerialComm m_serial_comm;
	// This JTAG tap is a bit faster, but the pins are hardwired.
	JTAGPortAVR m_jtag_port;
	XSVFPlayerJTAG m_xsvf_player;

	JTAGPort &jtagPort() { return m_jtag_port; }

protected:
	XSVFPlayer &xsvfPlayer() { return m_xsvf_player; }

public:
	PlayXSVFJTAGAVR(
		File &s,
		int stream_buffer_size);
	~PlayXSVFJTAGAVR() {}
	SerialComm &serialComm() { return m_serial_comm; }
};

#endif // PLAYXSVFJTAGAVR_H

#endif // ARDUINO_ARCH_AVR
