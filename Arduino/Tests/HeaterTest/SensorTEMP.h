#pragma once

#include "Arduino.h"
#include "Wire.h"
#include "HTS221.h"


/**
 * @file SensorTemp.h
 *
 * The NANO33BLE-Sensor HTS221, is an ultra-compact sensor that uses a polymer dielectric planar
 * capacitor structure capable of detecting relative humidity variations and temperature, returned as digital output on a serial interface.
 *
 * The humidity component probably needs some time to measure exact values
 *
 * @author Johannes Brunner
 *
 */

 // toDo: find right OFFSETS

class SensorTemp
{
private:

	HTS221 smeHumidity;

	const static uint8_t PIN_ENABLE_HTS = 4;
	const float TEMP_OFFSET = -2.0f;
	const float HUMI_OFFSET = 0.0f;

	float k = 0.5;
	float x[2];

	TwoWire* wireToUse_ = NULL;

public:
	virtual void init();
	virtual void stop();
	float* getSensorValues();
	void setWire(TwoWire* wireToUse_);
	bool writeRegister(byte slaveAddress, byte regToWrite, byte dataToWrite);
	byte readRegister(byte slaveAddress, byte regToRead);
	void enableHeater();
	void disableHeater();
	SensorTemp() {};
	~SensorTemp() {};
};

