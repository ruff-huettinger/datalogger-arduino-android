#pragma once

#include "Arduino.h"
#include "pretty_printer.h"
#include "ble/ble.h"

using namespace ble;

class BleConnection
{
private:
    bool _isConnecting = false;
    bool _isConnected = false;
    connection_handle_t _connection_handle;

public:
    bool isConnecting();
    bool isConnected();
    connection_handle_t getHandle();

    void setConnecting(bool state);
    void setConnected(bool state);
    void setHandle(connection_handle_t newHandle);

    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event);
    virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event);
};