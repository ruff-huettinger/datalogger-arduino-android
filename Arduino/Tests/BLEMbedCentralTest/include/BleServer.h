#pragma once

#include "Arduino.h"
#include "ble/ble.h"
#include "LedService.h"
#include "BleConnection.h"

class BleServer : public BleConnection
{
private:
    static constexpr const char *DEVICE_NAME = "EI_TX8_REP";
    UUID *REP_CHARACTERISTIC_UUID = new UUID(0xA005);

    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder *_adv_data_builder;
    LEDService *_led_service;

    GattServer *_server;

    void onGattServerData(const GattWriteCallbackParams *params);

public:
    BleServer();
    ~BleServer(){};

    void setServer(GattServer *newServer);
    void setCallbacks();

    void startAdvertising();

    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;
};