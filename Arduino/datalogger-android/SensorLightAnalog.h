#pragma once

#include "ISensor.h"

/**
 * @file SensorLightAnalog.h
 *
 * Measures the Analog Value given by the Bivar PLP5-2 lighting sensors attached to the egg shell
 *
 * @author Johannes Brunner
 *
 */

class SensorLightAnalog : public ISensor
{
private:
	uint32_t NUM_OF_LIGHT_READINGS = 500;
	float readValue(uint8_t pin);
public:
	virtual void init();
	virtual void stop();
	virtual void getSensorValue(measuring* value);

	SensorLightAnalog() {};
	~SensorLightAnalog() {};
};

