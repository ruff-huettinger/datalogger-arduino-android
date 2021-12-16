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

void BleServer::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    Serial.println("A device connected to me (Handy)");
    BleConnection::onConnectionComplete(event);
}

void BleServer::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    Serial.println("A device disconnected from me (Handy)");
    BleConnection::onDisconnectionComplete(event);
}

void BleServer::startAdvertising()
{
    Serial.println("Start Advertising");
    
}
