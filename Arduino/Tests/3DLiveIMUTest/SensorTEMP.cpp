#include "SensorTEMP.h"

void SensorTemp::init()
{
	//Initiate the Wire library and join the I2C bus

#ifndef DEBUG_BOARD
	Wire.begin();
	smeHumidity.setWire(&Wire);
	digitalWrite(PIN_ENABLE_HTS, HIGH);
#else
	Wire1.begin();
	smeHumidity.setWire(&Wire1);
#endif // DEBUG_BOARD

	//uint32_t startMillis = millis();
	//while ((millis() - startMillis) < 50) {};

	smeHumidity.begin();

	uint32_t delayMillis = millis();
	while ((millis() - delayMillis) < 50) {};
}

void SensorTemp::stop()
{
#ifndef DEBUG_BOARD
	Wire.end();
	digitalWrite(PIN_ENABLE_HTS, LOW);
#else
	Wire1.end();
#endif // DEBUG_BOARD
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

