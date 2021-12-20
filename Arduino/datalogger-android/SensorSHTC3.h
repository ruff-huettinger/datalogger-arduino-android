#pragma once

#include "ISensor.h"
#include "Wire.h"
#include "SparkFun_SHTC3.h"

/**
 * @file SensorSHTC3.h
 *
 * Implementation of Temperatur and Humidity Sensor SHTC3
 * Currently not used in Sensor-Egg
 *
 * @author Johannes Brunner
 *
 */


class SensorSHTC3 : public ISensor
{
private:
	SHTC3 mySHTC3_;
	SHTC3_Status_TypeDef result_;

public:
	void init();
	void stop();
	void getSensorValue(measuring* values);
	SensorSHTC3() {};
	~SensorSHTC3() {};
};

