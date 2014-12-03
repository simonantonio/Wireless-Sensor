
#include "Error.h"
#include "Configuration.h"

#include <Arduino.h>

Error::Error()
{
	errorControl = 0;//clear

	#ifdef ERROR_PIN
		pinMode(ERROR_PIN, OUTPUT);
	#endif
}

void Error::DisplayErrors()
{
	//flash a pin?
	if (errorControl == 0)
		return;

	#ifdef ERROR_PIN
		digitalWrite(ERROR_PIN, 1);
	#endif
	
}

void Error::SetError(ErrorStatus error)
{
	errorControl |= 1 << error;
}

void Error::ClearError(ErrorStatus error)
{
	errorControl &= ~(1 << error);
}

bool Error::CheckError(ErrorStatus error)
{
	int bit = errorControl & (1 << error);
	if (bit == error)
	return true;
	
	return false;
}
