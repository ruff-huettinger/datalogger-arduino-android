/*
  BLEMultiValueTest

  - Sending multiple sensor value in 1 BLE characteristic

  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * BLE-Device

  Created 15 02 2021
  By Johannes Brunner

  Src:
  https://forum.arduino.cc/t/arduino-nan-ble-33-how-to-send-data/632690/7
*/

#include <ArduinoBLE.h>

//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_SENSOR_DATA_SERVICE              "f1111a7f-0000-41f9-a127-aedf35a799b3"
#define BLE_UUID_MULTI_SENSOR_DATA                "f1111a7f-0003-41f9-a127-aedf35a799b3"

#define NUMBER_OF_SENSORS 4

union multi_sensor_data
{
    struct __attribute__((packed))
    {
        float values[NUMBER_OF_SENSORS];
    };
    uint8_t bytes[NUMBER_OF_SENSORS * sizeof(float)];
};

union multi_sensor_data multiSensorData;


//----------------------------------------------------------------------------------------------------------------------
// BLE
//----------------------------------------------------------------------------------------------------------------------

BLEService sensorDataService(BLE_UUID_SENSOR_DATA_SERVICE);
BLECharacteristic multiSensorDataCharacteristic(BLE_UUID_MULTI_SENSOR_DATA, BLERead, sizeof multiSensorData.bytes);


const int BLE_LED_PIN = LED_BUILTIN;
const int RSSI_LED_PIN = LED_PWR;


void setup()
{
    Serial.begin(9600);
    //while ( !Serial );

    pinMode(BLE_LED_PIN, OUTPUT);
    pinMode(RSSI_LED_PIN, OUTPUT);

    if (setupBleMode())
    {
        digitalWrite(BLE_LED_PIN, HIGH);
    }

    for (int i = 0; i < NUMBER_OF_SENSORS; i++)
    {
        multiSensorData.values[i] = i;
    }
}


void loop()
{
#define UPDATE_INTERVALL 50
    static long previousMillis = 0;

    // listen for BLE peripherals to connect:
    BLEDevice central = BLE.central();

    if (central)
    {
        Serial.print("Connected to central: ");
        Serial.println(central.address());

        while (central.connected())
        {
            unsigned long currentMillis = millis();
            if (currentMillis - previousMillis > UPDATE_INTERVALL)
            {
                previousMillis = currentMillis;

                Serial.print("Central RSSI: ");
                Serial.println(central.rssi());

                if (central.rssi() != 0)
                {
                    digitalWrite(RSSI_LED_PIN, LOW);

                    for (int i = 0; i < NUMBER_OF_SENSORS; i++)
                    {
                        multiSensorData.values[i] = multiSensorData.values[i] + 0.1;
                    }

                    multiSensorDataCharacteristic.writeValue(multiSensorData.bytes, sizeof multiSensorData.bytes);
                }
                else
                {
                    digitalWrite(RSSI_LED_PIN, HIGH);
                }
            }
        }

        Serial.print(F("Disconnected from central: "));
        Serial.println(central.address());
    }
}



bool setupBleMode()
{
    if (!BLE.begin())
    {
        return false;
    }

    // set advertised local name and service UUID:
    BLE.setDeviceName("Arduino Nano 33 BLE");
    BLE.setLocalName("EggCallback");
    BLE.setAdvertisedService(sensorDataService);

    // BLE add characteristics
    sensorDataService.addCharacteristic(multiSensorDataCharacteristic);

    // add service
    BLE.addService(sensorDataService);

    // set the initial value for the characeristic:
    multiSensorDataCharacteristic.writeValue(multiSensorData.bytes, sizeof multiSensorData.bytes);

    // start advertising
    BLE.advertise();

    return true;
}
