#include "PowerManager.h"

void PowerManager::init()
{
	initBattery();

	initAnalogLights();

	initSDModul();

	initExternHTS();
}

void PowerManager::initExternHTS()
{
	pinMode(PIN_ENABLE_HTS, OUTPUT);
	digitalWrite(PIN_ENABLE_HTS, HIGH);
}

void PowerManager::initSDModul()
{
	pinMode(PIN_ENABLE_SD_MODUL, OUTPUT);
	digitalWrite(PIN_ENABLE_SD_MODUL, HIGH);
}

void PowerManager::initAnalogLights()
{
	pinMode(PIN_READ_ANALOG_LIGHT_1, INPUT);
	pinMode(PIN_READ_ANALOG_LIGHT_2, INPUT);
	pinMode(PIN_ENABLE_ANALOG_LIGHT, OUTPUT);
	digitalWrite(PIN_ENABLE_ANALOG_LIGHT, HIGH);
}

void PowerManager::initBattery()
{
	pinMode(READ_BATTERY_PIN, INPUT);
	pinMode(ENABLE_BATTERY_MEASURE_PIN, OUTPUT);
	digitalWrite(ENABLE_BATTERY_MEASURE_PIN, HIGH);
}

uint16_t PowerManager::getAnalogAvg()
{
	uint32_t total = 0;
	digitalWrite(ENABLE_BATTERY_MEASURE_PIN, LOW);
	uint32_t timeBefMeasureStart = millis();
	while ((millis() - timeBefMeasureStart) < 1) {
		//idle
	}
	for (int i = 0; i < NUM_OF_BATTERY_READINGS; i++) {
		total += analogRead(READ_BATTERY_PIN);
		uint32_t timeBefNextMeasure = micros();
		while ((micros() - timeBefNextMeasure) < 1) {
			//idle
		}
	}

	digitalWrite(ENABLE_BATTERY_MEASURE_PIN, HIGH);
	return total / NUM_OF_BATTERY_READINGS;
}

float PowerManager::getBatteryFraction()
{
	uint16_t avg = getAnalogAvg();
	float avgInBounds = (float)constrain(avg, MIN_ANALOG_READ, MAX_ANALOG_READ);
	return mapfloat(avgInBounds, (float)MIN_ANALOG_READ, (float)MAX_ANALOG_READ, 0.0f, 1.0f);
}

void PowerManager::disableCurrentConsumers()
{
	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_STOPRX = 1;
	NRF_UART0->ENABLE = 0;

	*(volatile uint32_t*)0x40002FFC = 0;
	*(volatile uint32_t*)0x40002FFC;
	*(volatile uint32_t*)0x40002FFC = 1; //Setting up UART registers again due to a library issue
}

void PowerManager::enableOnBoardSensors() {
	digitalWrite(PIN_ENABLE_SENSORS_3V3, HIGH);
	digitalWrite(PIN_ENABLE_I2C_PULLUP, HIGH);
};

void PowerManager::enableHFCLK() {
	if (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {
		NRF_CLOCK->TASKS_HFCLKSTART = 1;
		while (NRF_CLOCK->EVENTS_HFCLKSTARTED == 0) {}
	}
}

void PowerManager::disableHFCLK()
{
	NRF_CLOCK->TASKS_HFCLKSTOP = 1;
	NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
}

void PowerManager::enableLFCLK()
{
	if (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {
		NRF_CLOCK->TASKS_LFCLKSTART = 1;
		while (NRF_CLOCK->EVENTS_LFCLKSTARTED == 0) {};
	}
}

void PowerManager::disableLFCLK()
{
	NRF_CLOCK->TASKS_LFCLKSTOP = 1;
	NRF_CLOCK->EVENTS_LFCLKSTARTED = 0;
}

void PowerManager::swapLFCLKSource()
{
	NRF_CLOCK->LFCLKSRC = 1;
}

void PowerManager::resetLFCLK()
{
	disableLFCLK();
	swapLFCLKSource();
	enableLFCLK();
}

float PowerManager::mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}
