/*
 * HTS221.h
 *
 * Created: 02/01/2015 20:50:30
 *  Author: smkk
 */


#ifndef HTS221_H_
#define HTS221_H_

#include <Arduino.h>

 //- NOT THE ORIGINAL LIBRARY - modified for choosing Wire or Wire1 (jb)


class HTS221
{
public:

	TwoWire* wireToUse_ = NULL;

	void setWire(TwoWire* wire);

	HTS221(void);
	bool begin(void);
	bool activate(void);
	bool deactivate(void);

	bool bduActivate(void);
	bool bduDeactivate(void);

	const double readHumidity(void);
	const double readTemperature(void);

private:

	bool storeCalibration(void);
	unsigned char _h0_rH, _h1_rH;
	unsigned int  _T0_degC, _T1_degC;
	unsigned int  _H0_T0, _H1_T0;
	unsigned int  _T0_OUT, _T1_OUT;
	double _temperature;
	double _humidity;
	uint8_t _address;

	byte readRegister(byte slaveAddress, byte regToRead);
	bool writeRegister(byte slaveAddress, byte regToWrite, byte dataToWrite);
};


extern HTS221 smeHumidity;


#endif /* HTS221_H_ */
