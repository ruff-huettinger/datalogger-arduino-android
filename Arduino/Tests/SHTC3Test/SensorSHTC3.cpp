#include "SensorSHTC3.h"

/*
void SensorSHTC3::init()
{
	//Initiate the Wire library and join the I2C bus

#ifndef DEBUG_BOARD
	Wire.begin();
#else
	Wire1.begin();
#endif // DEBUG_BOARD

	//uint32_t startMillis = millis();
	//while ((millis() - startMillis) < 50) {};

	mySHTC3.begin();

	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 50) {};
}

void SensorSHTC3::stop()
{
#ifndef DEBUG_BOARD
	//Wire.end();
	mySHTC3.sleep();
	Wire.end();
	digitalWrite(PIN_ENABLE_HTS, LOW);
#else
	Wire1.end();
#endif // DEBUG_BOARD
}

void SensorSHTC3::getSensorValue(measuring* values) {
	result = mySHTC3.update();  // Call "update()" to command a measurement, wait for measurement to complete, and update the RH and T members of the object
	float x;
	if (strcmp("temp", values->valueName) == 0) {
		x = mySHTC3.toDegC();
		x += TEMP_OFFSET;
	}
	else if (strcmp("humi", values->valueName) == 0) {
		x = mySHTC3.toPercent();
		x += HUMI_OFFSET;
	}
	values->sensorName = "SHTC3";
	values->value = x;
}

*/
