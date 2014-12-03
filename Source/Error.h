/*
	Primitve error handler - turns on an error light
*/

#ifndef _ERROR_H_
#define _ERROR_H_

enum ErrorStatus{
	WIFI_ERROR = 0x01,
	BMP_ERROR = 0x02,
	DHT_ERROR = 0x04,
	OTHER_ERROR = 0x08
};

class Error{
	public:
		Error();
		void DisplayErrors();
		void SetError(ErrorStatus error);
		void ClearError(ErrorStatus error);
		bool CheckError(ErrorStatus error);
	private:
		int errorControl;
};

#endif //_ERROR_H_