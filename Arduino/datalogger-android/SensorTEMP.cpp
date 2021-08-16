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

void SensorTemp::getSensorValue(measuring* values) {
	float x;
	if (strcmp("temp", values->valueName) == 0) {
		x = smeHumidity.readTemperature();
		x += TEMP_OFFSET;
	}
	else if (strcmp("humi", values->valueName) == 0) {
		x = smeHumidity.readHumidity();
		x += HUMI_OFFSET;
	}
	values->sensorName = "HTS221";
	values->value = x;
}

void SensorTemp::setWire(TwoWire* wire)
{
	wireToUse_ = wire;
}

