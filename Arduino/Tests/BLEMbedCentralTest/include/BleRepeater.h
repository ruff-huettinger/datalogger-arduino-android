#pragma once

#include "Arduino.h"
#include "ble/ble.h"
#include <events/mbed_events.h>
#include "mbed.h"
#include "BleClient.h"
#include "BleServer.h"
#include "BleData.h"
#include "pretty_printer.h"

using namespace ble;
using namespace mbed;

class BleRepeater : public ble::Gap::EventHandler
{
private:
    BLE *_ble;
    ble::Gap *_gap;
    inline static events::EventQueue *_event_queue;
    u_int8_t _led = 13;
    DigitalOut *led = new DigitalOut(p13);

    BleClient _client;
    BleServer _server;
    BleData _data;

    static void scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context);

    // GAP handlers (forwarded to client and server)
    void onConnectionComplete(const ble::ConnectionCompleteEvent &event); // client + server
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event); // client + server
    void onAdvertisingReport(const ble::AdvertisingReportEvent &event); // client

    // Asynchronous cbs
    void onInitialized(BLE::InitializationCompleteCallbackContext *event);

public:
    BleRepeater(){};
    ~BleRepeater(){};

    void init();

};