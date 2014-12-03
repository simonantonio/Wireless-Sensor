
#include <SFE_BMP180.h>
#include "Configuration.h"
#include "Datagram.h"

#ifndef BMP180_MANAGER_H_
#define _BMP180_MANAGER_H_

class BMP180_MANAGER
{
public:
	BMP180_MANAGER();
	void LoadDatagram(DATAGRAM &datagram);
	void SetDevice(SFE_BMP180 &device);
private:
	SFE_BMP180 pressure;
};

#endif //_BMP180_MANAGER_H_
