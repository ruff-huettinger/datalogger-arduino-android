#pragma once

#include <Wire.h>
#include "ISensor.h"
#include "SparkFunLSM9DS1.h" // sparkfun version

#define USING_SPARKFUN_LIB

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
	const static uint16_t IMU_INIT_DELAY = 50;

	LSM9DS1 imu_;

	int writeRegister(uint8_t slaveAddress, uint8_t address, uint8_t value);
public:
	void init();
	void stop();
	void getSensorValue(measuring* values);
	SensorMotion() {};
	~SensorMotion() {};
};

