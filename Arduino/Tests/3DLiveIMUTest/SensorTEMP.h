#pragma once

#include "ISensor.h"
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

class SensorTemp : public ISensor
{
private:

	const float TEMP_OFFSET = -2.0f;
	const float HUMI_OFFSET = 0.0f;

public:
	virtual void init();
	virtual void stop();
	virtual void getSensorValue(measuring* values);
	SensorTemp() {};
	~SensorTemp() {};
};

