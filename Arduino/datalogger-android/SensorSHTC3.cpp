#include "SensorSHTC3.h"

void SensorSHTC3::init()
{
	//Initiate the Wire library and join the I2C bus

	Wire.begin();

	// toDo: test pin enabling
	//digitalWrite(PIN_ENABLE_HTS, HIGH);

	mySHTC3.begin();

	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 50) {};
}

void SensorSHTC3::stop()
{
	mySHTC3.sleep();
	Wire.end();
	//digitalWrite(PIN_ENABLE_HTS, LOW);
}

void SensorSHTC3::getSensorValue(measuring* values) {
	result = mySHTC3.update();  // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object
	float x;
	if (strcmp("temp", values->valueName) == 0) {
		x = mySHTC3.toDegC();
	}
	else if (strcmp("humi", values->valueName) == 0) {
		x = mySHTC3.toPercent();
	}
	values->sensorName = "SHTC3";
	values->value = x;
}

