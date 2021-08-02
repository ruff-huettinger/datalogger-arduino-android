// Accelerometer.h

#ifndef _ACCELEROMETER_h
#define _ACCELEROMETER_h

#if defined(ARDUINO) && ARDUINO >= 100
#include "arduino.h"
#else
#include "WProgram.h"
#endif

class Acceleromter {
public:
	float getPitch();
	float getRoll();
	float getYaw();

	float* getQuaternion();

	void setup();

	void loop();

	void getMres();

	void getGres();

	void getAres();

	void readAccelData(int16_t* destination);

	void readGyroData(int16_t* destination);

	void readMagData(int16_t* destination);

	int16_t readTempData();

	void initLSM9DS1();

	void selftestLSM9DS1();

	void accelgyrocalLSM9DS1(float* dest1, float* dest2);

	void magcalLSM9DS1(float* dest1);

	void writeByte(uint8_t address, uint8_t subAddress, uint8_t data);

	uint8_t readByte(uint8_t address, uint8_t subAddress);

	void readBytes(uint8_t address, uint8_t subAddress, uint8_t count, uint8_t* dest);

	void MadgwickQuaternionUpdate(float ax, float ay, float az, float gx, float gy, float gz, float mx, float my, float mz);

	void magset();

	float* getEulers();
};

#endif

