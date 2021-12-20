#include "SensorSHTC3.h"

void SensorSHTC3::init()
{
	Wire.begin();

	mySHTC3_.begin();

	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 50) {};
}

void SensorSHTC3::stop()
{
	mySHTC3_.sleep();
	Wire.end();
}

void SensorSHTC3::getSensorValue(measuring* values) {
	result_ = mySHTC3_.update();
	float x;
	if (strcmp("temp", values->valueName) == 0) {
		x = mySHTC3_.toDegC();
	}
	else if (strcmp("humi", values->valueName) == 0) {
		x = mySHTC3_.toPercent();
	}
	values->sensorName = "SHTC3";
	values->value = x;
}

