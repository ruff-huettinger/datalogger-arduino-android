/*
	SystemOffButtonTest

	- After running in loop for some time, Arduino goes to SYSTEM-OFF Mode
	- A button can be used to wake up and reset the board
	- The power-consumption in different modes is measured using a multimeter


	The circuit:
	* Board: Arduino Nano 33 BLE Sense
	* Multimeter on GND and +3V3
	* Microswitch on GND and PIN D2

	Measurements:
	* System-Off mode: 11.3µA - 12.2µA
	* Running board: 925µA

	Created 15 09 2020
	By Johannes Brunner

	Sources:
	https://github.com/mristau/Arduino_nRF5x_lowPower

*/

#define LED_DEBUG

// Variables:

const u_int8_t INTERRUPT_PIN = 4;
const u_int16_t runningSeconds = 5;
u_int32_t cnt = 0;
bool wokeUp = true;
volatile bool intFlag = false;

void setup() {
	// toDo: Waking Up sometimes leads to Bootloader-Mode  
	Serial.begin(9600);
	delay(1000);
	Serial.println("RESET");

	// Init interrupt
	pinMode(INTERRUPT_PIN, INPUT_PULLUP);
	// Suprisingly also works in System-Off
	attachInterrupt(digitalPinToInterrupt(INTERRUPT_PIN), interruptRoutine, FALLING);


#ifdef LED_DEBUG
	digitalWrite(LED_PWR, LOW);
#else 
	digitalWrite(LED_PWR, LOW);
	digitalWrite(13, LOW);
#endif // LED_DEBUG
	Serial.println("SETUP OVER");
}

void loop() {

	// Log when coming from setup()
	if (wokeUp) {
		Serial.println("WOKE UP");
		wokeUp = false;
	}

	// Log the current counter
	Serial.println(cnt);

	// Log when interupt happened
	if (intFlag) {
		Serial.println("INTERUPT HAPPENED");
		intFlag = false;
	}

	// Go to System-off mode
	if (cnt >= runningSeconds) {
		Serial.println("SLEEP");
	#ifdef LED_DEBUG
		digitalWrite(13, LOW);
	#endif // LED_DEBUG
		NRF_POWER->SYSTEMOFF = 1;
	}

#ifdef LED_DEBUG
	digitalWrite(13, LOW);
#endif // LED_DEBUG
	cnt++;
	delay(1000);
}

void interruptRoutine() {
	intFlag = true;
}


// Functions from lib:

// not needed to wake up the arduino (toDo: why?)

void enableWakeupByInterrupt(uint32_t pin, uint32_t mode)
{
	NRF_GPIO->PIN_CNF[pin] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	switch (mode)
	{
	case HIGH:
	case RISING:
		NRF_GPIO->PIN_CNF[pin] |= ((uint32_t)GPIO_PIN_CNF_SENSE_High << GPIO_PIN_CNF_SENSE_Pos);
		break;
	case LOW:
	case FALLING:
		NRF_GPIO->PIN_CNF[pin] |= ((uint32_t)GPIO_PIN_CNF_SENSE_Low << GPIO_PIN_CNF_SENSE_Pos);
		break;
	default:
		return;
	}

	// Another attempt:
	// nrf_gpio_cfg_sense_input(INTERRUPT_PIN, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
}

void disableWakeupByInterrupt(uint32_t pin)
{
	NRF_GPIO->PIN_CNF[pin] &= ~((uint32_t)GPIO_PIN_CNF_SENSE_Msk);
	NRF_GPIO->PIN_CNF[pin] |= ((uint32_t)GPIO_PIN_CNF_SENSE_Disabled << GPIO_PIN_CNF_SENSE_Pos);
}
