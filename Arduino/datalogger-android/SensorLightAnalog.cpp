#include "SensorLightAnalog.h"

void SensorLightAnalog::init()
{
}

void SensorLightAnalog::stop()
{
}

void SensorLightAnalog::getSensorValue(measuring* value)
{
	if (strcmp("light2", value->valueName) == 0) {
		value->value = readValue(PIN_READ_ANALOG_LIGHT_1);
	}
	else {
		value->value = readValue(PIN_READ_ANALOG_LIGHT_2);
	}
	value->sensorName = "LIGHT";
}

float SensorLightAnalog::readValue(uint8_t pin)
{
	uint32_t total = 0;
	digitalWrite(PIN_ENABLE_ANALOG_LIGHT, LOW);
	uint32_t timeBefMeasureStart = millis();
	while ((millis() - timeBefMeasureStart) < 1) {
		//idle
	}
	for (int i = 0; i < NUM_OF_LIGHT_READINGS; i++) {
		total += analogRead(pin);
		uint32_t timeBefNextMeasure = micros();
		while ((micros() - timeBefNextMeasure) < 1) {
			//idle
		}
	}
	digitalWrite(PIN_ENABLE_ANALOG_LIGHT, HIGH);
	uint32_t avg = total / NUM_OF_LIGHT_READINGS;
	return (avg / 1024.0f) * 100.0f;
}
