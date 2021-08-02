/*
	SystemOnTest

	- After setup, Arduino switches between to SYSTEM-ON LOW_POWER and NORMAL MODE
	- The power-consumption in different modes is measured using a multimeter


	The circuit:
	* Board: Arduino Nano 33 BLE Sense
	* Multimeter on GND and +3V3

	Measurements:
	* LOW-POWER mode: ~5uA
	* Running board: 925uA

	Created 05 10 2020
	By Johannes Brunner

	Sources:
	https://github.com/mristau/Arduino_nRF5x_lowPower
	https://forum.arduino.cc/index.php?topic=699799.0

	https://devzone.nordicsemi.com/f/nordic-q-a/19276/deep-sleep-with-rtc-wakeup
	Quote: "If you need RAM to be retained in System On mode, you have to add an additional 30nA per 4kB RAM block that is used."
	toDo: Think about reducing current consumption even more through disabling RAM blocks
*/

#define LED_DEBUG

// Variables:

const u_int16_t RUNNING_SECONDS = 5;
const u_int16_t SLEEPING_SECONDS = 5;
const u_int16_t SETUPING_SECONDS = 3;
u_int32_t cnt = 0;
bool wokeUp = false;
volatile bool intFlag = false;


void setup() {
	// Green power led lights

	// toDo: fix returning to setup after sleep mode

	delay(SETUPING_SECONDS * 1000);

#ifdef LED_DEBUG
	digitalWrite(LED_PWR, LOW);
#else 
	digitalWrite(LED_PWR, LOW);
	digitalWrite(13, LOW);
#endif // LED_DEBUG
	disableCurrentConsumers();
}

void loop() {
	// Go to System-On LOW_POWER mode
	if (cnt >= RUNNING_SECONDS) {
	#ifdef LED_DEBUG
		digitalWrite(13, LOW);
	#endif //LED_DEBUG
		//NRF_POWER->TASKS_LOWPWR = 1;

		//------- SLEEP
		delay(SLEEPING_SECONDS * 1000);
		//------- WAKE UP
		cnt = 0;
	}

#ifdef LED_DEBUG
	else {
		digitalWrite(13, HIGH);
	}
#endif // LED_DEBUG
	cnt++;

	u_int32_t countMillis = millis();
	while ((millis() - 1000) < countMillis) {
		// idle with led off
	}
}

void disableCurrentConsumers() {
	digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
	digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW);

	//Disabling UART0 (saves around 300-500mA) - @Jul10199555 contribution

	NRF_UART0->TASKS_STOPTX = 1;
	NRF_UART0->TASKS_STOPRX = 1;
	NRF_UART0->ENABLE = 0;


	*(volatile uint32_t*)0x40002FFC = 0;
	*(volatile uint32_t*)0x40002FFC;
	*(volatile uint32_t*)0x40002FFC = 1; //Setting up UART registers again due to a library issue

}

// override the default main function to remove USB CDC feature
int main(void)
{
	init();
	initVariant();

	//remove USB CDC feature
	//#if defined(SERIAL_CDC)
	//  PluggableUSBD().begin();
	//  SerialUSB.begin(115200);
	//#endif

	setup();

	for (;;) {
		loop();
		if (arduino::serialEventRun) arduino::serialEventRun();
	}

	return 0;
}
