#pragma once

#include "config.h"
#include <Arduino.h>

/**
 * @file ISensor.h
 *
 * Interface for the different sensors used in the project
 *
 * @author Johannes Brunner
 *
 */

class ISensor
{
public:

	ISensor() {};
	virtual ~ISensor() {};

	// initialize the sensor
	virtual void init() = 0; // pure virtual

	// do a sensor measuring and fill the struct
	virtual void getSensorValue(measuring* values) = 0;  // pure virtual

	// de-activate the sensor to reduce current consumption
	virtual void stop() = 0; // pure virtual
};
