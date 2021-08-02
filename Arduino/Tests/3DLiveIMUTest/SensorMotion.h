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
	void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);

	void calibrate();

	SensorMotion() {};
	~SensorMotion() {};


	// Magnetometer registers
#define LSM9DS1M_ADDRESS  0x1E  //  Address of magnetometer

#define LSM9DS1M_OFFSET_X_REG_L_M   0x05
#define LSM9DS1M_OFFSET_X_REG_H_M   0x06
#define LSM9DS1M_OFFSET_Y_REG_L_M   0x07
#define LSM9DS1M_OFFSET_Y_REG_H_M   0x08
#define LSM9DS1M_OFFSET_Z_REG_L_M   0x09
#define LSM9DS1M_OFFSET_Z_REG_H_M   0x0A
#define LSM9DS1M_WHO_AM_I           0x0F  // should be 0x3D
#define LSM9DS1M_CTRL_REG1_M        0x20
#define LSM9DS1M_CTRL_REG2_M        0x21
#define LSM9DS1M_CTRL_REG3_M        0x22
#define LSM9DS1M_CTRL_REG4_M        0x23
#define LSM9DS1M_CTRL_REG5_M        0x24
#define LSM9DS1M_STATUS_REG_M       0x27
#define LSM9DS1M_OUT_X_L_M          0x28
#define LSM9DS1M_OUT_X_H_M          0x29
#define LSM9DS1M_OUT_Y_L_M          0x2A
#define LSM9DS1M_OUT_Y_H_M          0x2B
#define LSM9DS1M_OUT_Z_L_M          0x2C
#define LSM9DS1M_OUT_Z_H_M          0x2D
#define LSM9DS1M_INT_CFG_M          0x30
#define LSM9DS1M_INT_SRC_M          0x31
#define LSM9DS1M_INT_THS_L_M        0x32
#define LSM9DS1M_INT_THS_H_M        0x33
};

