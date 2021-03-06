#include "SensorLight.h"

void SensorLight::init()
{
	if (!APDS.begin()) {
		DEBUG_PRINTLN("Error initializing APDS9960 sensor.");
	}
	//APDS.proximityAvailable();
	//APDS.colorAvailable();
}

void SensorLight::stop()
{
	APDS.end();
}

void SensorLight::getSensorValue(measuring* values) {
	int r = 0;
	int g = 0;
	int b = 0;
	int x = 0;

	// give sensor a max time for each measuring 
	u_int32_t lightStartTime = millis();

	if (strcmp("prox", values->valueName) == 0) {
		while (!APDS.proximityAvailable() && (millis() - lightStartTime) < 100) {};
		x = APDS.readProximity();
	}
	else if (strcmp("light1", values->valueName) == 0) {
		while (!APDS.colorAvailable() && (millis() - lightStartTime) < 100) {};
		APDS.readColor(r, g, b, x);
	}
	values->sensorName = "APDS";
	values->value = (float)x;
}

