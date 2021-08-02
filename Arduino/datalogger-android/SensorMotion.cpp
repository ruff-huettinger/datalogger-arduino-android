#include "SensorMotion.h"

//Sparkfun version:

#ifdef USING_SPARKFUN_LIB



void SensorMotion::getSensorValue(measuring* values)
{
	float x, y, z;

	// give sensor a max of 20ms for each measuring 
	u_int32_t motionStartTime = millis();

	char* name = strdup(values->valueName);
	u_int8_t nameLength = strlen(name);

	char id = name[nameLength - 1];
	name[nameLength - 1] = '\0';

	if (strcmp("acc", name) == 0) {
		while (!imu.accelAvailable() && (millis() - motionStartTime) < 50) {};
		imu.readAccel();

		if (id == 'X') {
			values->value = (imu.calcAccel(imu.ax));
		}
		else if (id == 'Y') {
			values->value = (imu.calcAccel(imu.ay));
		}
		else if (id == 'Z') {
			values->value = (imu.calcAccel(imu.az));
		}
	}
	else if (strcmp("gyro", name) == 0) {
		while (!imu.gyroAvailable() && (millis() - motionStartTime) < 50) {};
		imu.readGyro();

		if (id == 'X') {
			values->value = (imu.calcGyro(imu.gx));
		}
		else if (id == 'Y') {
			values->value = (imu.calcGyro(imu.gy));
		}
		else if (id == 'Z') {
			values->value = (imu.calcGyro(imu.gz));
		}
	}
	else if (strcmp("mag", name) == 0) {
		while (!imu.magAvailable() && (millis() - motionStartTime) < 100) {};
		imu.readMag();

		if (id == 'X') {
			values->value = (imu.calcMag(imu.mx));
		}
		else if (id == 'Y') {
			values->value = (imu.calcMag(imu.my));
		}
		else if (id == 'Z') {
			values->value = (imu.calcMag(imu.mz));
		}
	}

	values->sensorName = "LSM9DS1";

#ifdef DEBUG_SENSOR_INFO
	DEBUG_PRINT(values->valueName);
	DEBUG_PRINT(": ");
#ifndef DEBUG_SENSOR_TIMES
	DEBUG_PRINTLN(values->value);
#else
	DEBUG_PRINT(values->value);
	DEBUG_PRINT(" @ ");
	DEBUG_PRINT(millis());
	DEBUG_PRINTLN("ms");
#endif // DEBUG_SENSOR_TIMES
#endif // DEBUG_SENSOR_INFO

	free(name);
}

void SensorMotion::init()
{
	Wire1.begin();
	Wire1.setClock(400000);

	if (imu.begin(0x6B, 0X1E, Wire1) == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
	{
		DEBUG_PRINTLN("Failed to communicate with LSM9DS1.");
		DEBUG_PRINTLN("Double-check wiring.");
		DEBUG_PRINTLN("Default settings in this sketch will " \
			"work for an out of the box LSM9DS1 " \
			"Breakout, but may need to be modified " \
			"if the board jumpers are.");
		return;
	}
	uint32_t startMillis = millis();
	while ((millis() - IMU_INIT_DELAY) < startMillis) {
		//wait
	}
}

void SensorMotion::stop()
{
	writeRegister(0x1e, 0x22, 0x03);
	writeRegister(0x6b, 0x10, 0x00);
	writeRegister(0x6b, 0x20, 0x00);

	Wire1.end();
}

int SensorMotion::writeRegister(uint8_t slaveAddress, uint8_t address, uint8_t value)
{
	Wire1.beginTransmission(slaveAddress);
	Wire1.write(address);
	Wire1.write(value);
	if (Wire1.endTransmission() != 0) {
		return 0;
	}
	return 1;
}


#else

// Arduino version:

void SensorMotion::init()
{
	if (!IMU.begin()) {
		DEBUG_PRINTLN("Failed to initialize IMU!");
		//while (1);
	}
}

void SensorMotion::stop()
{
	IMU.end();
}

void SensorMotion::getSensorValue(measuring* values)
{
	float x, y, z;

	// give sensor a max of 20ms for each measuring 
	u_int32_t motionStartTime = millis();

	char* name = strdup(values->valueName);
	u_int8_t nameLength = strlen(name);

	char id = name[nameLength - 1];
	name[nameLength - 1] = '\0';

	if (strcmp("acc", name) == 0) {
		while (!IMU.accelerationAvailable() && millis() < motionStartTime + 50) {};
		IMU.readAcceleration(x, y, z);
	}
	else if (strcmp("gyro", name) == 0) {
		while (!IMU.gyroscopeAvailable() && millis() < motionStartTime + 50) {};
		IMU.readGyroscope(x, y, z);
	}
	else if (strcmp("mag", name) == 0) {
		while (!IMU.magneticFieldAvailable() && millis() < motionStartTime + 100) {};
		IMU.readMagneticField(x, y, z);
	}

	if (id == 'X') {
		values->value = x;
	}
	else if (id == 'Y') {
		values->value = y;
	}
	else if (id == 'Z') {
		values->value = z;
	}

	values->sensorName = "LSM9DS1";

#ifdef DEBUG_SENSOR_INFO
	DEBUG_PRINT(values->valueName);
	DEBUG_PRINT(": ");
#ifndef DEBUG_SENSOR_TIMES
	DEBUG_PRINTLN(values->value);
#else
	DEBUG_PRINT(values->value);
	DEBUG_PRINT(" @ ");
	DEBUG_PRINT(millis());
	DEBUG_PRINTLN("ms");
#endif // DEBUG_SENSOR_TIMES
#endif // DEBUG_SENSOR_INFO

	free(name);
}


#endif // USING_SPARKFUN_LIB

