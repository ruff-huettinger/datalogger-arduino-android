#pragma once

#include "SensorLight.h"
#include "SensorMotion.h";
#include "SensorTemp.h"
#include "SensorLightAnalog.h"
#include "SensorSHTC3.h"
#include "PowerManager.h"
#include "config.h"


/**
 * @file SensorManager.h
 *
 * - Interface between controller and sensors
 * - Stores sensor measurings and returns them to the controller
 * - Gets active sensors from config
 *
 * @author Johannes Brunner
 *
 */


class SensorManager
{
private:

	static bool measurementActive;

public:

	SensorManager() {};

	~SensorManager() {};

	// creates sensor-objects, starts measurings and fills the whole values-array
	void getSensorValues(measuring* values);

	// returns the number of measurings according to the active sensors defined in config.h
	uint8_t getDataLength();

	// prints sensor values to serial
	void printSensorValues(measuring* dataToPrint, uint8_t numOfSensors);

	void addIDs(measuring* values);

	static bool getMeasurementActive();

	static void setMeasurementActive(bool active);

};

