#include "BleConnection.h"

bool BleConnection::isConnecting()
{
    return _isConnecting;
};

bool BleConnection::isConnected()
{
    return _isConnected;
};

connection_handle_t BleConnection::getHandle()
{
    return _connection_handle;
};

void BleConnection::setConnecting(bool state)
{
    _isConnecting = state;
};

void BleConnection::setConnected(bool state)
{
    _isConnected = state;
};

void BleConnection::setHandle(connection_handle_t newHandle)
{
    _connection_handle = newHandle;
};

void BleConnection::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    setConnected(true);
    setConnecting(false);
    setHandle(event.getConnectionHandle());
};

void BleConnection::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    setConnected(false);
    setConnecting(false);
};