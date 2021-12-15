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
        _client.onConnectionComplete(event);
    }

    else if (event.getOwnRole() == connection_role_t(connection_role_t::PERIPHERAL))
    {
        _server.onConnectionComplete(event);
    }
}

void BleRepeater::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    // use connection handle to find decision (server <> client)
    Serial.println("I disconnected from egg");
    //_gap->startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    if (event.getConnectionHandle() == _client.getHandle())
    {
        _client.onDisconnectionComplete(event);
    }
    else if (event.getConnectionHandle() == _server.getHandle())
    {
        _server.onDisconnectionComplete(event);
    }
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