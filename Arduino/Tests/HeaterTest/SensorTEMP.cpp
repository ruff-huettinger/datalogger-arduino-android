#include "SensorTEMP.h"

void SensorTemp::init()
{
	//Initiate the Wire library and join the I2C bus

	wireToUse_->begin();
	smeHumidity.setWire(wireToUse_);
	digitalWrite(PIN_ENABLE_HTS, LOW);

	smeHumidity.begin();

	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 50) {};
}

void SensorTemp::stop()
{
	wireToUse_->end();
	digitalWrite(PIN_ENABLE_HTS, HIGH);
}


float* SensorTemp::getSensorValues() {
	x[0] = smeHumidity.readTemperature();
	x[0] += 2.0;

	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 100) {};

	x[1] = smeHumidity.readHumidity();
	return x;
}

void SensorTemp::setWire(TwoWire* wire)
{
	wireToUse_ = wire;
}

bool SensorTemp::writeRegister(byte slaveAddress, byte regToWrite, byte dataToWrite)
{
	if (wireToUse_ != NULL) {
		wireToUse_->beginTransmission(slaveAddress);

		if (!wireToUse_->write(regToWrite)) {
			return false;
		}
		if (!wireToUse_->write(dataToWrite)) {
			return false;
		}

		uint8_t errorNo = wireToUse_->endTransmission(); //Stop transmitting
		return (errorNo == 0);
	}
	else {
		return 1;
	}
}

byte SensorTemp::readRegister(byte slaveAddress, byte regToRead)
{
	if (wireToUse_ != NULL) {
		wireToUse_->beginTransmission(slaveAddress);
		wireToUse_->write(regToRead);
		wireToUse_->endTransmission(false); //endTransmission but keep the connection active

		wireToUse_->requestFrom(slaveAddress, 1); //Ask for 1 byte, once done, bus is released by default

		while (!wireToUse_->available()); //Wait for the data to come back
		return wireToUse_->read(); //Return this one byte
	}
	else {
		return -1;
	}
}

void SensorTemp::enableHeater()
{
	int32_t data = 0;

	data = readRegister(0x5F, 0x21);

	data &= ~(0b1 << 1);
	data |= (0b1 << 1);

	writeRegister(0x5F, 0x21, (uint8_t)data);
}

