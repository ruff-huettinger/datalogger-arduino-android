#pragma once

#include "Arduino.h"
#include "Ble.h"

#define NUM_OF_CHARS 1

class BleData
{
private:
    DiscoveredCharacteristic *_clientChars[NUM_OF_CHARS];
    GattCharacteristic *_serverChars[NUM_OF_CHARS];

public:
    BleData(){};
    ~BleData(){};

    void addNewCharacteristic(DiscoveredCharacteristic *newChar);
};
