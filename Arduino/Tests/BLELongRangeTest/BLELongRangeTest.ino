/*
  BLELongRangeTest

  - Testing long range ble with mbed

  The circuit:
  * Board: Arduino Nano 33 BLE Sense
  * Scanning BLE Device
  
  Created 04 11 2021
  By Johannes Brunner

  Src:
  https://forum.arduino.cc/t/nano-33-ble-nrf52840-ble-long-range-pairing-mbed-ble-api-adafruit-lib/644684

*/

/*--------INCLUDES--------*/
#include "ble/BLE.h"
#include <driver/CordioHCIDriver.h>

/*------------------------*/

using namespace mbed;       // Enable us to call mbed functions without "mbed::"

/*------------GLOBAL VARIABLES-------------*/
const static char DEVICE_NAME[] = "Egg Demo";                    // Device name when detected on Bluetooth
static events::EventQueue event_queue(10 * EVENTS_EVENT_SIZE);  // Create Event queue
static const uint16_t MAX_ADVERTISING_PAYLOAD_SIZE = 59;        // Advertising payload parameter
const ble::phy_set_t CodedPHY(ble::phy_t::LE_CODED);            // Creating a Coded Phy set
/*-----------------------------------------*/





/*------------------------------Bluetooth Device class------------------------------*/
class LRDemo : public ble::Gap::EventHandler {
public:
    /* Class constructor */
    LRDemo(BLE& ble, events::EventQueue& event_queue) :
        _ble(ble),                                      // BLE API Class
        _event_queue(event_queue),
        _adv_handle(ble::INVALID_ADVERTISING_HANDLE),   // Advertising parameter
        _adv_data_builder(_adv_buffer) { }              // Advertising parameter

    /* Class destructor */
    ~LRDemo() {
        if (_ble.hasInitialized()) {
            _ble.shutdown();
        }
    }

    void start() {
        _ble.gap().setEventHandler(this);               // Assign GAP events to this class
        _ble.init(this, &LRDemo::on_init_complete);     // Initialize Bluetooth
        _event_queue.dispatch_forever();                // Wait for event forever
    }

private:
    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext* params) {
        if (params->error != BLE_ERROR_NONE) {
            Serial.println("Ble initialization failed.");
            return;
        }
        //CordioHCIDriver& ble_cordio_get_hci_driver();
        //NRF_RADIO->TXPower = 0;
        Serial.println("Ble initialized.");

        /* Create advertising parameters and payload */
        ble::AdvertisingParameters adv_parameters(
            //ble::advertising_type_t::CONNECTABLE_NON_SCANNABLE_UNDIRECTED,    // Advertising Type here : connectable non scannable undirected = connectable with exetended advertising
            ble::advertising_type_t::CONNECTABLE_UNDIRECTED,    // Advertising Type here : connectable non scannable undirected = connectable with exetended advertising
            ble::adv_interval_t(ble::millisecond_t(500)),           // Min Advertising time in ms
            ble::adv_interval_t(ble::millisecond_t(500)),           // Max Advertising time in ms
            false                                                   // Legacy PDU : Needed to be OFF in Long Range Mode
        );

        
        adv_parameters.setPhy(ble::phy_t::LE_CODED, ble::phy_t::LE_CODED);  // Set Advertising radio modulation to LE_CODED Phy (=Long Range)
        adv_parameters.setTxPower(8);                                       // Set radio output power to 8dbm (max) 
        //adv_parameters.setTxPower(127);                                       // Set radio output power to 8dbm (max) 
        _ble.gap().setPreferredPhys(&CodedPHY, &CodedPHY);                  // Set preferred connection phy to LE_CODED (=long range)
        

        if (_adv_handle == ble::INVALID_ADVERTISING_HANDLE) {       // Create advertising set with parameters defined before
            _ble.gap().createAdvertisingSet(
                &_adv_handle,
                adv_parameters);
        }
        _adv_data_builder.clear();
        _adv_data_builder.setFlags();
        _adv_data_builder.setName(DEVICE_NAME);                     // Set Bluetooth device name

        /* Setup advertising */
        _ble.gap().setAdvertisingParameters(_adv_handle, adv_parameters);
        _ble.gap().setAdvertisingPayload(_adv_handle, _adv_data_builder.getAdvertisingData());

        /* Start advertising */
        _ble.gap().startAdvertising(_adv_handle);
        Serial.println("Start advertising...");
    }

    void onConnectionComplete(const ble::ConnectionCompleteEvent& event) {
        Serial.println("Device connected");
    }

    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&) {
        Serial.println("Device disconnected");
        _ble.gap().startAdvertising(_adv_handle);
        Serial.println("Start advertising...");
    }


private:
    /* Class variables declaration*/
    BLE& _ble;
    events::EventQueue& _event_queue;

    uint8_t _adv_buffer[MAX_ADVERTISING_PAYLOAD_SIZE];  // Advertising parameters
    ble::advertising_handle_t _adv_handle;              //
    ble::AdvertisingDataBuilder _adv_data_builder;      //
};
/*----------------------------------------------------------------------------------*/

/** Schedule processing of events from the BLE middleware in the event queue. */
void schedule_ble_events(BLE::OnEventsToProcessCallbackContext* context) {
    event_queue.call(Callback<void()>(&context->ble, &BLE::processEvents));
}

/*====================== MAIN CODE ======================*/

void setup() {
    /* Setup Debugging */

    //sd_ble_gap_tx_power_set(8);
    //sd_ble_gap_tx_power_set(1, 1, +8);
    Serial.begin(9600);
    while (!Serial);

    /* Low Power */
    digitalWrite(LED_PWR, LOW);                 // Turn off power LED
    digitalWrite(PIN_ENABLE_SENSORS_3V3, LOW);  // Turn off sensors
    NRF_POWER->DCDCEN = 0x00000001;             // Enable DCDC
    /*           */

    BLE& ble = BLE::Instance();                 // Create the BLE object in order to use BLE_API function
    ble.onEventsToProcess(schedule_ble_events); // Set event schedule

    LRDemo demo(ble, event_queue);              // Create LRDemo Object
    demo.start();                               // Start Bluetooth Long Range 
}

void loop() {}

/*=======================================================*/
