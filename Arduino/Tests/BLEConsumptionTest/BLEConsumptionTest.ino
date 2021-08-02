/*
  BLEConsumptionTest

  - A test to determine the power consumption of Arduino-BLE-lib
  - The power-consumption in different modes is measured using a multimeter


  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Multimeter on GND and +3V3

  Measurements:
  * BLE commented out: ~5uA
  * BLE begin called: 925uA
  * BLE end called: 600uA

  Created 09 12 2020
  By Johannes Brunner
  
*/

#define LED_DEBUG

#include <ArduinoBLE.h>

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

// Variables:

const u_int16_t RUNNING_SECONDS = 5;
const u_int16_t SLEEPING_SECONDS = 5;
const u_int16_t SETUPING_SECONDS = 3;
u_int32_t cnt = 0;
bool wokeUp = false;
volatile bool intFlag = false;


void setup() {

  delay(SETUPING_SECONDS * 1000);

#ifdef LED_DEBUG
  digitalWrite(LED_PWR, LOW);
#else 
  digitalWrite(LED_PWR, LOW);
  digitalWrite(13, LOW);
#endif // LED_DEBUG
  disableCurrentConsumers();

  // begin initialization

  /*
    if (!BLE.begin()) {
        Serial.println("starting BLE failed!");

        while (1);
    }

    

    // set the local name peripheral advertises
    BLE.setLocalName("LEDCallback");
    // set the UUID for the service this peripheral advertises
    BLE.setAdvertisedService(ledService);

    // add the characteristic to the service
    ledService.addCharacteristic(switchCharacteristic);

    // add service
    BLE.addService(ledService);

    // assign event handlers for connected, disconnected to peripheral
    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    // assign event handlers for characteristic
    switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);
    // set an initial value for the characteristic
    switchCharacteristic.setValue(0);

    // start advertising
    BLE.advertise();

    Serial.println(("Bluetooth device active, waiting for connections..."));
    */
}

void loop() {
 // BLE.poll();
  // Go to System-On LOW_POWER mode
  if (cnt >= RUNNING_SECONDS) {
  #ifdef LED_DEBUG
    digitalWrite(13, LOW);
  #endif //LED_DEBUG
    //NRF_POWER->TASKS_LOWPWR = 1;
    BLE.end();
    //NRF_CLOCK->TASKS_HFCLKSTOP = 1;
    //------- SLEEP
    delay(SLEEPING_SECONDS * 1000);
    //------- WAKE UP
    BLE.begin();
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

void blePeripheralConnectHandler(BLEDevice central) {
    // central connected event handler
    Serial.print("Connected event, central: ");
    Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
    // central disconnected event handler
    Serial.print("Disconnected event, central: ");
    Serial.println(central.address());
}

void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
    // central wrote new value to characteristic, update LED
    Serial.print("Characteristic event, written: ");

    if (switchCharacteristic.value()) {
        Serial.println("LED on");
        digitalWrite(LED_BUILTIN, HIGH);
    }
    else {
        Serial.println("LED off");
        digitalWrite(LED_BUILTIN, LOW);
        BLE.end();
        delay(1000000000);
    }
}

void disableCurrentConsumers() {
  digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);
  digitalWrite(PIN_ENABLE_I2C_PULLUP, LOW);

  //Disabling UART0 (saves around 300-500�A) - @Jul10199555 contribution

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
