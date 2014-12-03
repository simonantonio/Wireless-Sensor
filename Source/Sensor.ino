#include <SPI.h>
#include <Wire.h>
#include <DHT.h>
#include <PinChangeInt.h>

#include <SFE_BMP180.h>

#include <RHReliableDatagram.h>
#include <RH_NRF24.h>

#include <Time.h>

#include "Datagram.h"
#include "BMP180_Manager.h"
#include "Error.h"

/*
	Author:
		S.Antonio 2014-12-03
	Description:
		Board Uses Radio Head library for NRF24L01+ Wireless Transceiver
		NRF24
			CS																	PD7 [D7]
			CE																	PB0 [D8]

		BMP180																	i2c
		DHT22																	PD4 [D4]
		Sparkfun Weather Meters (SEN-08942)
			Rain																PD5 [D5]
			Pulse (add resister to high side R7)	(anometer)					PD2 [D2]
			Wind Vane (wire to ADC3 [IR])										ADC3 [A3]

		Pulse 2	(add resister to high side R12)									PB1	[D9]
		Hall																	PD6 [D6]

		Light Dependant Resister (10k)											ADC2 [A2]
		
		Optional, 
			Battery Monitoring													ADC0 [A0]
			Battery Read (Pull High)											ADC1 [A1]
			Power Shut Down (High = ON, Low = OFF)								PB2 [D10]
				Will last x seconds before caps run out of power in low power mode		-- TODO calculations and watch dog to recharge caps for standby
			TSOP4838 (IR 32kHz carrier)											ADC3 [A3]
	Notes:
		Target Arduino Pro Mini 3.3v 8MHz - Board is based of ATMEGA 328 running at 8MHz
		Either populate the D24V3F3 (pololu 3.3 regulator) or supply clean 3.3v source and add jumper to P2 (PWR_JMP)
		Typically either use the Weather Meter, or the IR sensor, but not both together.

	Libraries:
		http://www.airspayce.com/mikem/arduino/RadioHead/index.html				NRF24
		https://github.com/adafruit/DHT-sensor-library							DHT22
		https://github.com/sparkfun/BMP180_Breakout/archive/master.zip			BMP180
		http://playground.arduino.cc/Code/Time									Time
	Changes:
		N/A
	TODO:
		ppm oscillator drift offset
		remote time set
		remote query
		move user configs to seperate header
*/

//USER Configuration here

//IR or WEATHER - set false for IR sensing

#define IR_PIN A3
#define WS_WIND_DIRECTION A3
#define WS_RAIN 5
#define WS_WIND_SPEED 2

//LDR Pin
#define LDR_PIN A2

//Wireless Setup
#define NRF24_CLIENT_ADDRESS 1
#define NRF24_SERVER_ADDRESS 2

// Singleton instance of the radio driver
RH_NRF24 driver(8,7);

// Class to manage message delivery and receipt, using the driver declared above
RHReliableDatagram manager(driver, NRF24_CLIENT_ADDRESS);

//DHT22 Setup
DHT dht(4, DHT22);

//BMP180 Setup
BMP180_MANAGER bmp;
SFE_BMP180 pressure;

//Error Handler
Error err;

volatile uint8_t latest_interrupted_pin;
volatile uint8_t interrupt_count[20] = { 0 }; // 20 possible arduino pins

volatile unsigned long debounce_min = 0xffffffff;
volatile unsigned long debounce_last = 0;

void quicfunc() {
	//Test this debounce code
	long thisTime = micros() - debounce_last;
	debounce_last = micros();
	if (thisTime > 500)
	{
		latest_interrupted_pin = PCintPort::arduinoPin;
		interrupt_count[latest_interrupted_pin]++;

		if (thisTime < debounce_min)
		{
			debounce_min = thisTime;
		}

	}

	//latest_interrupted_pin = PCintPort::arduinoPin;
	//interrupt_count[latest_interrupted_pin]++;
}

void setup()
{
	if (!manager.init())
	{
		err.SetError(WIFI_ERROR);
	}
	if (!pressure.begin())
	{
		err.SetError(BMP_ERROR);
	}
	else
	{
		bmp.SetDevice(pressure);
	}

	dht.begin();

	//Setup INPUT pints (external pullups)
	pinMode(WS_RAIN, INPUT);
	pinMode(WS_WIND_DIRECTION, INPUT);
	pinMode(WS_WIND_SPEED, INPUT);

	PCintPort::attachInterrupt(WS_RAIN, &quicfunc, FALLING);  // add more attachInterrupt code as required
        PCintPort::attachInterrupt(WS_WIND_SPEED, &quicfunc, FALLING);  // add more attachInterrupt code as required
}

uint8_t nrf24_send_data[] = "BMP_T:{0}\nBMP_P:{1}\nDHT_T:{2}\nDHT_H:{3}\nWS_D:{4}\nWS_R:{5}\nWS_S:{6}\nLDR:{7}\nBAT:{8}\nP2_C{9}\nH_C:{10}";

// Dont put this on the stack:
uint8_t nrf24_response_buf[RH_NRF24_MAX_MESSAGE_LEN];

void loop()
{
	//TODO change this to SLEEP + WATCHDOG + INTERUPTS
	delay(5000);//every 5 seconds + what ever delay the folloing libs need

	DATAGRAM datagram; //clear it out

	//init datagram values to 0 where counting
	datagram.Pulse_Count = 0;
	datagram.WS_Rain_Count = 0;
	datagram.WS_Wind_Speed_Count = 0;

	//get date and time for datagram
	datagram.Day =  day();
	datagram.Hour = hour();
	datagram.Minute = minute();
	datagram.Second = second();
	datagram.Day = day();
	datagram.Month = month();
	datagram.Year = year();

	//test casting datagram to message
	//is apparently naughty will need to be tweaked/replaced later on
	uint8_t* msg = reinterpret_cast<uint8_t(&)[sizeof datagram]>(datagram);

	// Send a message to manager_server
	//if (manager.sendtoWait(nrf24_send_data, sizeof(nrf24_send_data), NRF24_SERVER_ADDRESS))
	if (manager.sendtoWait(msg, sizeof(msg), NRF24_SERVER_ADDRESS))
	{
		// Now wait for a reply from the server
		uint8_t len = sizeof(nrf24_response_buf);
		uint8_t from;
		if (manager.recvfromAckTimeout(nrf24_response_buf, &len, 2000, &from))
		{
			//server address (from)
			//from

			//response string
			//(char*)buf;

			//Add parsing for commands here
			//Check if we need to set the time 
			//Run any remote queries here
		}
		else
		{
			//No reply, is nrf24_reliable_datagram_server running?
			err.SetError(WIFI_ERROR);
		}
	}
	else
	{
		//sendtoWait failed
		err.SetError(WIFI_ERROR);
	}
	
	//DHT22
	datagram.DHT_Humidity = dht.readHumidity();
	datagram.DHT_Tempurature_C = dht.readTemperature();


	//BMP180
	bmp.LoadDatagram(datagram);
	
	//LDR
	int ldr_value;
	ldr_value = analogRead(LDR_PIN);

	//TODO - SPARKFUN WEATHER SENSORS
	if (IS_WEATHER_SENSOR)
	{
		//DO weather sensor (read A
		int wind_direction;
		wind_direction = analogRead(WS_WIND_DIRECTION);

		//Loop each pin TODO - OPTIMISE 
		//Possible issue with bouncing - TODO Debounce
		uint8_t i;
		for (i = 0; i < 20; i++) 
		{
			if (interrupt_count[i] != 0) 
			{
				if (i == WS_RAIN)
					datagram.WS_Rain_Count = interrupt_count[i];
				if (i == WS_WIND_SPEED)
					datagram.WS_Wind_Speed_Count = interrupt_count[i];
			}
		}
	}
	else
	{
		//IR sensing
	}
}
