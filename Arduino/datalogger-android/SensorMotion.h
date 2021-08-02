#pragma once

#define USING_SPARKFUN_LIB

#include <Wire.h>
#include "ISensor.h"

#ifdef USING_SPARKFUN_LIB
#include "SparkFunLSM9DS1.h" // sparkfun version
#else 
#include <Arduino_LSM9DS1.h>
#endif // USING_SPARKFUN_LIB

// toDo: think about sensor times and queries

/**
 * @file SensorMotion.h
 *
 * The NANO33BLE-Sensor LSM9DS1, is a 3 - axis accelerometer, 3 - axis gyroscope and 3 - axis magnetometer.
 * Results are in G (earth gravity), degrees/second and uT (micro Tesla).
 * @author Johannes Brunner
 *
 */



class SensorMotion : public ISensor
{

private:

#ifdef USING_SPARKFUN_LIB
	LSM9DS1 imu;
#endif

	const static uint16_t IMU_INIT_DELAY = 50;

public:
	virtual void init();
	virtual void stop();
	int writeRegister(uint8_t slaveAddress, uint8_t address, uint8_t value);
	virtual void getSensorValue(measuring* values);
	SensorMotion() {};
	~SensorMotion() {};
};

