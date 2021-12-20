#pragma once

#include "ISensor.h"
#include "Arduino_APDS9960.h"

/**
 * @file SensorLight.h
 *
 * The NANO33BLE-Sensor is a APDS9960, it senses gesture, color, ambience illumination and proximity
 *
 * @author Johannes Brunner
 *
 */

class SensorLight : public ISensor
{
private:

public:
	void init();
	void stop();
	void getSensorValue(measuring* values);
	SensorLight() {};
	~SensorLight() {};
};


