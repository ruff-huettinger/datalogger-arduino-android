/*
  BLEMbedCentralTest

  - A test to build a central device with mbed functions
  - Based on the example  


  The circuit:
  * Board: Arduino Nano 33 BLE Sense (central)
  * Egg: (peripheral hosting "EggCallback"-Service)

  Created 23 11 2021
  By Johannes Brunner

  // toDo: test forwarding of data
  
*/

#include "Arduino.h"
#include <events/mbed_events.h>
#include "mbed.h"
#include "ble/ble.h"
#include "pretty_printer.h"
#include "LEDService.h"
#include "GattClientProcess.cpp"

using namespace mbed;
using namespace ble;

#define printf(...) Serial.printf(__VA_ARGS__)

class BLERepeater : public ble::Gap::EventHandler, public GattClient::EventHandler
{
    typedef BLERepeater Self;
    typedef DiscoveredCharacteristic::Properties_t Properties_t;

    typedef struct
    {
        ble::advertising_type_t type;
        ble::adv_interval_t min_interval;
        ble::adv_interval_t max_interval;
    } DemoAdvParams_t;

    typedef struct
    {
        ble::scan_interval_t interval;
        ble::scan_window_t window;
        ble::scan_duration_t duration;
        bool active;
    } DemoScanParam_t;

    const DemoScanParam_t my_scan_param = {ble::scan_interval_t(160), ble::scan_window_t(160), ble::scan_duration_t(0), false};
    static constexpr const char *DEVICE_NAME = "EI_TX8_REP";

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

    // General variables:
    BLE *_ble;
    ble::Gap *_gap;
    inline static events::EventQueue *_event_queue;
    u_int8_t _led = 13;
    bool isConnected = false;
    DigitalOut *led = new DigitalOut(p13);

    // Gatt variables:
    //ble::impl::GattClient *_client;

    GattClient *_client;
    //ble *_client;
    ble::connection_handle_t _connection_handle;
    //DiscoveredCharacteristicNode *_characteristics;
    //DiscoveredCharacteristicNode *_it;
    GattAttribute::Handle_t _descriptor_handle;

public:
    BLERepeater()
    {
    }
    ~BLERepeater() {}

    void begin()
    {
        Serial.println("Starting...");

        _ble = &BLE::Instance();

        _gap = &_ble->gap();

        _event_queue = new events::EventQueue(10 * EVENTS_EVENT_SIZE);

        _client = &_ble->gattClient();

        _ble->onEventsToProcess(schedule_ble_events);

        /* handle gap events */
        _gap->setEventHandler(this);

        ble_error_t error = _ble->init(this, &BLERepeater::onInitialized);

        if (error)
        {
            print_error(error, "Error returned by BLE::init");
            return;
        }

        _ble->gattServer().onDataWritten(this, &BLERepeater::onGattServerData);

        //_event_queue->call_every(500, this, &MyDemo::blink);
        _event_queue->dispatch_forever();
    }

    void scan()
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
    }

    void findService()
    {
        _client->onDataWritten().add(as_cb(&Self::when_descriptor_written));
        _client->onHVX().add(as_cb(&Self::when_characteristic_changed));

        _client->onServiceDiscoveryTermination(as_cb(&BLERepeater::when_service_discovery_ends));
        ble_error_t error = _client->launchServiceDiscovery(
            _connection_handle,
            as_cb(&BLERepeater::when_service_discovered),
            as_cb(&BLERepeater::when_characteristic_discovered));

        if (error)
        {
            printf("Error %u returned by _client->launchServiceDiscovery.\r\n", error);
            return;
        }

        // register as a handler for GattClient events
        _client->setEventHandler(this);

        // this might not result in a new value but if it does we will be informed through
        // an call in the event handler we just registered
        _client->negotiateAttMtu(_connection_handle);

        printf("Client process started: initiate service discovery.\r\n");
    }

    ////---------- Callbacks -------------------

    void onInitialized(BLE::InitializationCompleteCallbackContext *event)
    {
        Serial.println("ble init complete");
        scan();
    }

    void onGattServerData(const GattWriteCallbackParams *params) {}

    virtual void onAdvertisingReport(const ble::AdvertisingReportEvent &event)
    {
        if (event.getPeerAddress() == txAddress)
        {
            Serial.println("Scan found egg");

            /*
            //Use this for scanning!
            Serial.println(event.getRssi());
            return;
            */

            if (isConnected)
            {
                return;
            }
            else
            {
                // Connect
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
                isConnected = true;
            }
        }
    }

    // ---- GAP event handler

    void onConnectionComplete(const ble::ConnectionCompleteEvent &event)
    {
        Serial.println("I connected to egg");
        _connection_handle = event.getConnectionHandle();
        findService();
    }

    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &)
    {
        Serial.println("I disconnected from egg");
        _gap->startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    }

    // GATT cbs

    /**
     * Called when the CCCD has been written.
     */
    void when_descriptor_written(const GattWriteCallbackParams *event)
    {
    }

    /**
     * Print the updated value of the characteristic.
     *
     * This function is called when the server emits a notification or an
     * indication of a characteristic value the client has subscribed to.
     *
     * @see GattClient::onHVX()
     */
    void when_characteristic_changed(const GattHVXCallbackParams *event)
    {
    }

    /**
     * Handle services discovered.
     *
     * The GattClient invokes this function when a service has been discovered.
     *
     * @see GattClient::launchServiceDiscovery
     */
    void when_service_discovered(const DiscoveredService *discovered_service)
    {
        // print information of the service discovered
        printf("Service discovered: value = ");
        //print_uuid(discovered_service->getUUID());
        printf(", start = %u, end = %u.\r\n",
               discovered_service->getStartHandle(),
               discovered_service->getEndHandle());
    }

    void when_service_discovery_ends(ble::connection_handle_t connection_handle)
    {
        Serial.println("Gatt end cb triggered");
        /*
        if (!_characteristics)
        {
            printf("No characteristics discovered, end of the process.\r\n");
            return;
        }

        printf("All services and characteristics discovered, process them.\r\n");

        // reset iterator and start processing characteristics in order
        _it = NULL;
        _event_queue->call(mbed::callback(this, &Self::process_next_characteristic));
        */
    }

    /**
     * Handle characteristics discovered.
     *
     * The GattClient invoke this function when a characteristic has been
     * discovered.
     *
     * @see GattClient::launchServiceDiscovery
     */
    void when_characteristic_discovered(const DiscoveredCharacteristic *discovered_characteristic)
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

        // add the characteristic into the list of discovered characteristics

        /*
        bool success = add_characteristic(discovered_characteristic);
        if (!success)
        {
            printf("Error: memory allocation failure while adding the discovered characteristic.\r\n");
            _client->terminateServiceDiscovery();
            stop();
            return;
        }
        */
    }

    // ----- Helper
    template <typename ContextType>
    FunctionPointerWithContext<ContextType> as_cb(
        void (Self::*member)(ContextType context))
    {
        return makeFunctionPointer(this, member);
    }

    /**
     * Print the value of a UUID.
     */
    static void print_uuid(const UUID &uuid)
    {
        const uint8_t *uuid_value = uuid.getBaseUUID();

        // UUIDs are in little endian, print them in big endian
        for (size_t i = 0; i < uuid.getLen(); ++i)
        {
            printf("%02X", uuid_value[(uuid.getLen() - 1) - i]);
        }
    }

    /**
     * Print the value of a characteristic properties.
     */
    static void print_properties(const Properties_t &properties)
    {
        const struct
        {
            bool (Properties_t::*fn)() const;
            const char *str;
        } prop_to_str[] = {
            {&Properties_t::broadcast, "broadcast"},
            {&Properties_t::read, "read"},
            {&Properties_t::writeWoResp, "writeWoResp"},
            {&Properties_t::write, "write"},
            {&Properties_t::notify, "notify"},
            {&Properties_t::indicate, "indicate"},
            {&Properties_t::authSignedWrite, "authSignedWrite"}};

        printf("[");
        for (size_t i = 0; i < (sizeof(prop_to_str) / sizeof(prop_to_str[0])); ++i)
        {
            if ((properties.*(prop_to_str[i].fn))())
            {
                printf(" %s", prop_to_str[i].str);
            }
        }
        printf(" ]");
    }

    // ---- EventQ functions

    void blink()
    {
        Serial.println("blink");
        if (led->read() == 0)
        {
            led->write(1);
        }
        else
        {
            led->write(0);
        }
    }

    static void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
    {
        _event_queue->call(Callback<void()>(&context->ble, &BLE::processEvents));
    }
};

//-----------------------------------------------------------------------------------
//events::EventQueue MyDemo::*_event_queue = nullptr;

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;
    thread_sleep_for(1000);

    BLERepeater demo;
    demo.begin();
}

void loop() {}
