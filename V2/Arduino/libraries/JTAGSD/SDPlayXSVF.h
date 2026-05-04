
#ifndef PLAYXSVF_H
#define PLAYXSVF_H

//class Stream;
class SerialComm;
class XSVFPlayer;

class PlayXSVF
{
protected:
	virtual SerialComm &serialComm() = 0;
	virtual XSVFPlayer &xsvfPlayer() = 0;

public:
	PlayXSVF() {}
	~PlayXSVF() {}

	virtual int play();
  virtual void show_progress(int progress);
 	void printAvailableRAM();
};


//void printAvailableRAM(File &s);
void printAvailableRAM(SerialComm &s);
int availableRAM();

#endif // PLAYXSVF_H
