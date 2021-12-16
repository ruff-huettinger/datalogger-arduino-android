#pragma once

#include "Arduino.h"
#include "pretty_printer.h"
#include "ble/ble.h"
#include "BleConnection.h"
#include "ble/DiscoveredCharacteristic.h"
#include "ble/DiscoveredService.h"
#include "BleData.h"

using namespace ble;

class BleClient : public GattClient::EventHandler, public BleConnection
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

    GattAttribute::Handle_t _descriptor_handle;
    DiscoveredCharacteristic *_ledCharacteristic;

    void discoverService();

    template <typename ContextType>

    FunctionPointerWithContext<ContextType> convertToCallback(
        void (BleClient::*member)(ContextType context));

    void when_characteristic_discovered(const DiscoveredCharacteristic *discovered_characteristic);
    void when_service_discovery_ends(ble::connection_handle_t connection_handle);
    void when_service_discovered(const DiscoveredService *discovered_service);
    void when_descriptor_written(const GattWriteCallbackParams *event);
    void when_characteristic_changed(const GattHVXCallbackParams *event);

public:
    BleClient(){};
    ~BleClient(){};

    void setClient(GattClient *newClient);
    
    void startScanning();

    const uint8_t *getEggAdress();

    void onAdvertisingReport(const ble::AdvertisingReportEvent &event);
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;
};