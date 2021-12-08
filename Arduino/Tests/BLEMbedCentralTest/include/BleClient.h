#pragma once

#include "Arduino.h"
#include "pretty_printer.h"
#include "ble/ble.h"

using namespace ble;

class BleClient : public ble::Gap::EventHandler
{
private:
    typedef struct
    {
        scan_interval_t interval;
        scan_window_t window;
        scan_duration_t duration;
        bool active;
    } DemoScanParam_t;

    const DemoScanParam_t my_scan_param = {scan_interval_t(160), scan_window_t(160), scan_duration_t(0), false};

    const uint8_t eggAddress[6] = {
        0x58,
        0xCC,
        0xEA,
        0xDE,
        0x06,
        0x93};

    const uint8_t txAddress[6] = {
        0x60,
        0xAD,
        0x33,
        0x22,
        0x9D,
        0xE6};

    GattClient *_client;
    ble::Gap *_gap;

    bool _isConnecting = false;
    bool _isConnected = false;

    ble::connection_handle_t _connection_handle;
    GattAttribute::Handle_t _descriptor_handle;
    DiscoveredCharacteristic *_ledCharacteristic;

public:
    BleClient(){};
    ~BleClient(){};

    void setClient(GattClient *newClient, ble::Gap *newGap);

    void startScanning();

    const uint8_t *getEggAdress();

    void onAdvertisingReport(const ble::AdvertisingReportEvent &event);
};