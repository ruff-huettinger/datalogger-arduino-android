#include "BleData.h"

void BleData::addNewCharacteristic(DiscoveredCharacteristic *newChar)
{
    _clientChars[0] = newChar;
};
