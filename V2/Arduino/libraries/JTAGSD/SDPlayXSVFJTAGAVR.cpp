#ifdef ARDUINO_ARCH_AVR

#include "SDPlayXSVFJTAGAVR.h"

PlayXSVFJTAGAVR::PlayXSVFJTAGAVR(
	File &s,
	int stream_buffer_size)
: PlayXSVF()
, m_serial_comm(s, stream_buffer_size)
, m_jtag_port()
, m_xsvf_player(serialComm(), jtagPort())
{
}

#endif // ARDUINO_ARCH_AVR
