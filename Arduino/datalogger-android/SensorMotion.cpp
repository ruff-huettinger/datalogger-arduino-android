#include "SensorMotion.h"

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
		while (!imu_.accelAvailable() && (millis() - motionStartTime) < 50) {};
		imu_.readAccel();

		if (id == 'X') {
			values->value = (imu_.calcAccel(imu_.ax));
		}
		else if (id == 'Y') {
			values->value = (imu_.calcAccel(imu_.ay));
		}
		else if (id == 'Z') {
			values->value = (imu_.calcAccel(imu_.az));
		}
	}
	else if (strcmp("gyro", name) == 0) {
		while (!imu_.gyroAvailable() && (millis() - motionStartTime) < 50) {};
		imu_.readGyro();

		if (id == 'X') {
			values->value = (imu_.calcGyro(imu_.gx));
		}
		else if (id == 'Y') {
			values->value = (imu_.calcGyro(imu_.gy));
		}
		else if (id == 'Z') {
			values->value = (imu_.calcGyro(imu_.gz));
		}
	}
	else if (strcmp("mag", name) == 0) {
		while (!imu_.magAvailable() && (millis() - motionStartTime) < 100) {};
		imu_.readMag();

		if (id == 'X') {
			values->value = (imu_.calcMag(imu_.mx));
		}
		else if (id == 'Y') {
			values->value = (imu_.calcMag(imu_.my));
		}
		else if (id == 'Z') {
			values->value = (imu_.calcMag(imu_.mz));
		}
	}

	values->sensorName = "LSM9DS1";
	free(name);
}

void SensorMotion::init()
{
	Wire1.begin();
	Wire1.setClock(400000);

	if (imu_.begin(0x6B, 0X1E, Wire1) == false) // with no arguments, this uses default addresses (AG:0x6B, M:0x1E) and i2c port (Wire).
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

