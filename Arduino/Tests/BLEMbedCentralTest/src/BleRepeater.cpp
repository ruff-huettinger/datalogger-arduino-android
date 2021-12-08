#include "BleRepeater.h"

void BleRepeater::init()
{
    Serial.println("Starting...");

    _ble = &BLE::Instance();

    _gap = &_ble->gap();

    _event_queue = new events::EventQueue(10 * EVENTS_EVENT_SIZE);

    _client.setClient(&_ble->gattClient(), _gap);

    _server.setServer(&_ble->gattServer());
    _server.setCallbacks();

    _ble->onEventsToProcess(scheduleBleEvents);

    /* handle gap events */
    _gap->setEventHandler(this);

    ble_error_t error = _ble->init(this, &BleRepeater::onInitialized);

    if (error)
    {
        print_error(error, "Error returned by BLE::init");
        return;
    }

    //_event_queue->call_every(500, this, &MyDemo::blink);
    _event_queue->dispatch_forever();
}

void BleRepeater::scheduleBleEvents(BLE::OnEventsToProcessCallbackContext *context)
{
    _event_queue->call(Callback<void()>(&context->ble, &BLE::processEvents));
}

void BleRepeater::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    if (event.getOwnRole() == connection_role_t(connection_role_t::CENTRAL))
    {
        Serial.println("I connected as Central (to Egg)");
        //_client.onConnectionComplete();
    }

    else if (event.getOwnRole() == connection_role_t(connection_role_t::PERIPHERAL))
    {
        Serial.println("A device connected to me (Handy)");
        //_server.onConnectionComplete();
    }

    //_connection_handle = event.getConnectionHandle();
}

void BleRepeater::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &)
{
    // use connection handle to find decision (server <> client)
    Serial.println("I disconnected from egg");
    //_gap->startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
}

void BleRepeater::onInitialized(BLE::InitializationCompleteCallbackContext *event)
{
    Serial.println("ble init complete");
    _client.startScanning();
}

void BleRepeater::onAdvertisingReport(const ble::AdvertisingReportEvent &event)
{
    _client.onAdvertisingReport(event);
};