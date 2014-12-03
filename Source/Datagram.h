
#include "Configuration.h"
/*
	Binary packet to transmit to server containing sensor data
*/

#ifndef _DATAGRAM_H_
#define _DATAGRAM_H_

#ifdef IS_WEATHER_SENSOR
	#define FLAG 0xff
#else
	#define FLAG 0x00
#endif

struct DATAGRAM
{
	//Date Time
	uint8_t  Hour;
	uint8_t  Minute;
	uint8_t  Second;
	uint8_t  Day;
	uint8_t  Month;     // Jan is month 0
	uint8_t  Year;      // the Year minus 1900  

	//DHT22
	float DHT_Humidity;
	float DHT_Tempurature_C;

	//BMP180
	double BMP_Tempurature_C;
	double BMP_Absolute_Pressure_MB;
	double BMP_Pressure_Relative_MB;
	double BMP_Altitude;

	//0xff for weather sensor 0x00 for IR
	//byte Flag = FLAG;

	//Weather Station
#ifdef IS_WEATHER_SENSOR
	uint8_t  WS_Wind_Speed_Count;//since last datagram
	uint8_t  WS_Rain_Count;//since last datagram
	int WS_Direction;
#else
	//IR
	//TODO
#endif

	uint8_t  Pulse_Count;

	//LDR
	int LDR_Value;
};


#endif
