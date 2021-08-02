#pragma once

#include "ISensor.h"
#include "Wire.h"

#include "SparkFun_SHTC3.h" // Click here to get the library: http://librarymanager/All#SparkFun_SHTC3


/**
 * @file SensorSHTC3.h
 * 
 * A new sensor for temp and humi
 *
 *
 * @author Johannes Brunner
 *
 */


class SensorSHTC3 : public ISensor
{
private:

	SHTC3 mySHTC3;
	SHTC3_Status_TypeDef result;

public:
	virtual void init();
	virtual void stop();
	virtual void getSensorValue(measuring* values);
	SensorSHTC3() {};
	~SensorSHTC3() {};
};

