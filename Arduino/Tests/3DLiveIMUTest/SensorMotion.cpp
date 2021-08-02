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

void SensorMotion::calibrate()
{
	float mRes = 4.0 / 32768.0;

	uint8_t data[6]; // data array to hold mag x, y, z, data
	uint16_t ii = 0, sample_count = 0;
	int32_t mag_bias[3] = { 0, 0, 0 };
	int16_t mag_max[3] = { 0, 0, 0 }, mag_min[3] = { 0, 0, 0 };

	// configure the magnetometer-enable temperature compensation of mag data
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG1_M, 0x80 | 2 << 5 | 4 << 2); // select x,y-axis mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG2_M, 0 << 5); // select mag full scale
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG3_M, 0x00); // continuous conversion mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG4_M, 2 << 2); // select z-axis mode
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_CTRL_REG5_M, 0x40); // select block update mode

	mag_bias[0] = (501.46 / 1000) / mRes;
	mag_bias[1] = (79.22 / 1000) / mRes;
	mag_bias[2] = (15.75 / 1000) / mRes;

	//write biases to accelerometermagnetometer offset registers as counts);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_X_REG_L_M, (int16_t)mag_bias[0] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_X_REG_H_M, ((int16_t)mag_bias[0] >> 8) & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Y_REG_L_M, (int16_t)mag_bias[1] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Y_REG_H_M, ((int16_t)mag_bias[1] >> 8) & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Z_REG_L_M, (int16_t)mag_bias[2] & 0xFF);
	writeByte(LSM9DS1M_ADDRESS, LSM9DS1M_OFFSET_Z_REG_H_M, ((int16_t)mag_bias[2] >> 8) & 0xFF);
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
	calibrate();
	startMillis = millis();
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

void SensorMotion::writeByte(uint8_t address, uint8_t subAddress, uint8_t data)
{
	Wire1.beginTransmission(address);  // Initialize the Tx buffer
	Wire1.write(subAddress);           // Put slave register address in Tx buffer
	Wire1.write(data);                 // Put data in Tx buffer
	Wire1.endTransmission();           // Send the Tx buffer
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

