uint8_t ENABLE_PIN = 5;
uint8_t READ_PIN = 15;


void setup() {
	while (!Serial);
	pinMode(ENABLE_PIN, OUTPUT);
	pinMode(READ_PIN, INPUT);
	digitalWrite(ENABLE_PIN, HIGH);
	delay(100);
};

void loop() {

	Serial.println("Measuring begins");

	Serial.print(getAnalogAvg(1, 1, 1));
	Serial.print("  |  ");
	delay(500);
	Serial.print(getAnalogAvg(10, 1, 1));
	Serial.print("  |  ");
	delay(500);
	Serial.print(getAnalogAvg(100, 1, 1));
	Serial.print("  |  ");
	delay(500);
	Serial.print(getAnalogAvg(1000, 10, 10));
	Serial.print("  |  ");
	delay(500);
	Serial.println(getAnalogAvg(10000, 100, 10));
	delay(3000);
};



uint16_t getAnalogAvg(uint32_t numOfReadings, uint16_t beginDelay, uint16_t microDelay)
{
	uint32_t total = 0;
	digitalWrite(ENABLE_PIN, LOW);
	uint32_t timeBefMeasureStart = millis();
	while ((millis() - timeBefMeasureStart) < beginDelay) {
		//idle
	}
	for (int i = 0; i < numOfReadings; i++) {
		total += analogRead(READ_PIN);
		uint32_t timeBefNextMeasure = micros();
		while ((micros() - timeBefNextMeasure) < microDelay) {
			//idle
		}
	}

	digitalWrite(ENABLE_PIN, HIGH);
	return (total / numOfReadings);
}
