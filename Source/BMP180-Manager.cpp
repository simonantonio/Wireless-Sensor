
#include "BMP180_Manager.h"

BMP180_MANAGER::BMP180_MANAGER()
{

}

void BMP180_MANAGER::LoadDatagram(DATAGRAM &datagram)
{
	char status;
	//double T, P, p0, a;
	status = pressure.startTemperature();

	//Add to seperate function//class
	//pass in datagram object
	//have it be populated 
	if (status != 0)
	{
		// Wait for the measurement to complete:
		delay(status);

		status = pressure.getTemperature(datagram.BMP_Tempurature_C);
		if (status != 0)
		{
			//double temp_f = (9.0 / 5.0) * datagram.BMP_Tempurature_C + 32.0;
			
			status = pressure.startPressure(3);//high resolution messurement
			if (status != 0)
			{
				// Wait for the measurement to complete:
				delay(status);

				status = pressure.getPressure(datagram.BMP_Absolute_Pressure_MB, datagram.BMP_Tempurature_C);

				if (status != 0)
				{
					//Absolute pressure
					//datagram.BMP_Absolute_Pressure_MB
					
					//absolute pressure in inHg
					//double abs_pressure_inHg = datagram.BMP_Absolute_Pressure_MB * 0.0295333727;
					
					//Relative (sea-level) pressure
					datagram.BMP_Pressure_Relative_MB = pressure.sealevel(datagram.BMP_Absolute_Pressure_MB, ALTITUDE); // we're at {HOME LOCATION ALTITUDE}
					
					//relative (sea-level) pressure in inHg
					//datagram.BMP_Pressure_Relative_MB * 0.029533372;

					datagram.BMP_Altitude = pressure.altitude(datagram.BMP_Absolute_Pressure_MB, datagram.BMP_Pressure_Relative_MB);
					//Computed altitude meters
					//datagram.BMP_Altitude 

					//feet
					//datagram.BMP_Altitude * 3.28084;
				}
			}
		}
	}
}
void BMP180_MANAGER::SetDevice(SFE_BMP180 &device)
{
	pressure = device;
}
