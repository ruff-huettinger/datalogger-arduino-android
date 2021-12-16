#include "BleClient.h"

void BleClient::setClient(GattClient *newClient)
{
    _client = newClient;
};

void BleClient::startScanning()
{
    ble_error_t error = _gap->setScanParameters(
        ble::ScanParameters(
            ble::phy_t::LE_1M, // scan on the 1M PHY
            my_scan_param.interval,
            my_scan_param.window,
            my_scan_param.active));
    if (error)
    {
        print_error(error, "Error caused by Gap::setScanParameters");
        return;
    }

    /* start scanning and attach a callback that will handle advertisements
		 * and scan requests responses */

    _gap->startScan(my_scan_param.duration);

    Serial.println("started scanning");

    // Callback: BleRepeater::OnAdvertisingReport
};

const uint8_t *BleClient::getEggAdress()
{
    return eggAddress;
}

void BleClient::onAdvertisingReport(const ble::AdvertisingReportEvent &event)
{
    //if (event.getPeerAddress() == getEggAdress())
    if (event.getPeerAddress() == txAddress)
    {
        Serial.println("Scan found egg");

        /*
        //Use this for scanning!
        Serial.println(event.getRssi());
        return;
        */

        if (isConnecting() || isConnected())
        {
            return;
        }

        else
        {
            // Connect
            setConnecting(true);
            ble_error_t error = _gap->connect(
                event.getPeerAddressType(),
                event.getPeerAddress(),
                ble::ConnectionParameters() // use the default connection parameters
            );
            if (error)
            {
                print_error(error, "Error caused by Gap::connect");
                /* since no connection will be attempted end the mode */
                return;
            }

            ble_error_t error1 = _gap->stopScan();
            if (error)
            {
                print_error(error1, "Error caused by Gap::stopScan");
            }
        }
    }
};

void BleClient::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    Serial.println("I connected as Central (to Egg)");
    BleConnection::onConnectionComplete(event);
    discoverService();
}

void BleClient::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    Serial.println("I disconnected as Central (from Egg)");
    BleConnection::onDisconnectionComplete(event);
}

void BleClient::discoverService()
{
    _client->onDataWritten().add(convertToCallback(&BleClient::when_descriptor_written));
    _client->onHVX().add(convertToCallback(&BleClient::when_characteristic_changed));

    _client->onServiceDiscoveryTermination(convertToCallback(&BleClient::when_service_discovery_ends));
    ble_error_t error = _client->launchServiceDiscovery(
        getHandle(),
        convertToCallback(&BleClient::when_service_discovered),
        convertToCallback(&BleClient::when_characteristic_discovered));

    if (error)
    {
        printf("Error %u returned by _client->launchServiceDiscovery.\r\n", error);
        return;
    }

    // register as a handler for GattClient events
    _client->setEventHandler(this);

    // this might not result in a new value but if it does we will be informed through
    // an call in the event handler we just registered
    _client->negotiateAttMtu(getHandle());

    printf("Client process started: initiate service discovery.\r\n");
};

template <typename ContextType>
FunctionPointerWithContext<ContextType> BleClient::convertToCallback(
    void (BleClient::*member)(ContextType context))
{
    return makeFunctionPointer(this, member);
}

//// ---------------------
// GATT cbs

void BleClient::when_descriptor_written(const GattWriteCallbackParams *event)
{
}

void BleClient::when_characteristic_changed(const GattHVXCallbackParams *event)
{
}

/**
     * Handle services discovered.
     *
     * The GattClient invokes this function when a service has been discovered.
     *
     * @see GattClient::launchServiceDiscovery
     */
void BleClient::when_service_discovered(const DiscoveredService *discovered_service)
{
    // print information of the service discovered
    printf("Service discovered: value = ");
    //print_uuid(discovered_service->getUUID());
    printf(", start = %u, end = %u.\r\n",
           discovered_service->getStartHandle(),
           discovered_service->getEndHandle());

    //GattService *x = discovered_service;
    //toDo: think about discoverdService and Forwarding of data
}

void BleClient::when_service_discovery_ends(ble::connection_handle_t connection_handle)
{
    Serial.println("Service discovery end cb triggered");

    const byte a[1] = {1};

    //GATT_OP_WRITE_CMD

    _ledCharacteristic->getValueHandle();

    //_client->write(GattClient::GATT_OP_WRITE_REQ, _connection_handle, _ledCharacteristic->getValueHandle(), 1, a);

    _ledCharacteristic->write(1, a);

    Serial.print("LED Char uuid: ");
    print_uuid(_ledCharacteristic->getUUID());
    Serial.println();

    Serial.println((int)_ledCharacteristic->getLastHandle());

    // toDo: call a function of BLERepeater to invoke advertising on repeater
    // toDo: How? Callback? Object? 

    //startAdvertising();
}

/**
     * Handle characteristics discovered.
     *
     * The GattClient invoke this function when a characteristic has been
     * discovered.
     *
     * @see GattClient::launchServiceDiscovery
     */
void BleClient::when_characteristic_discovered(const DiscoveredCharacteristic *discovered_characteristic)
{
    // print characteristics properties
    printf("\tCharacteristic discovered: uuid = ");
    print_uuid(discovered_characteristic->getUUID());
    printf(", properties = ");
    print_properties(discovered_characteristic->getProperties());
    printf(
        ", decl handle = %u, value handle = %u, last handle = %u.\r\n",
        discovered_characteristic->getDeclHandle(),
        discovered_characteristic->getValueHandle(),
        discovered_characteristic->getLastHandle());

    UUID ledUUID("100A"); // toDo: compare on LED

    if (discovered_characteristic->getLastHandle() == 65535)
    {
        Serial.println("compare works");
        //memcpy(_ledCharacteristic, *discovered_characteristic, sizeof(DiscoveredCharacteristic));
        //_ledCharacteristic = new (std::nothrow) DiscoveredCharacteristic(*discovered_characteristic);
        _data.addNewCharacteristic(new (std::nothrow) DiscoveredCharacteristic(*discovered_characteristic));
    }
}
