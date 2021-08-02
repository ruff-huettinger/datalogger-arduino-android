
uint8_t ENABLE_PIN = 4;

int main(void)
{
	init();
	initVariant();

#ifdef DEBUG_SERIAL
	//remove USB CDC feature
#if defined(SERIAL_CDC)
	PluggableUSBD().begin();
	SerialUSB.begin(115200);
#endif
#endif // DEBUG_SERIAL
	setup();
	disableCurrentConsumers();
	while (true) {
		loop();
	}
	return 0;
}

void setup() {
	pinMode(ENABLE_PIN, OUTPUT);

	delay(5000);

	digitalWrite(ENABLE_PIN, LOW);
	delay(5000);

	digitalWrite(ENABLE_PIN, HIGH);
	delay(5000);

}

void loop() {}

void disableCurrentConsumers()
{
	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_STOPRX = 1;
	NRF_UART0->ENABLE = 0;

	*(volatile uint32_t*)0x40002FFC = 0;
	*(volatile uint32_t*)0x40002FFC;
	*(volatile uint32_t*)0x40002FFC = 1; //Setting up UART registers again due to a library issue
}