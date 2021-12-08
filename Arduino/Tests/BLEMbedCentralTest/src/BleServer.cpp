#include "BleServer.h"

BleServer::BleServer()
{
    _adv_data_builder = new ble::AdvertisingDataBuilder(_adv_buffer);
}

void BleServer::setServer(GattServer *newServer)
{
    Serial.println("server set");
    _server = newServer;
};

void BleServer::setCallbacks()
{
    _server->onDataWritten(this, &BleServer::onGattServerData);
}

void BleServer::onGattServerData(const GattWriteCallbackParams *params)
{
    Serial.println("BleServer::onGattServerData triggered");
}