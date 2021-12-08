#include "BleClient.h"

void BleClient::setClient(GattClient *newClient, ble::Gap *newGap)
{
    _client = newClient;
    _gap = newGap;
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
    if (event.getPeerAddress() == getEggAdress())
    {
        Serial.println("Scan found egg");

        /*
        //Use this for scanning!
        Serial.println(event.getRssi());
        return;
        */

        if (_isConnecting || _isConnected)
        {
            return;
        }

        else
        {
            // Connect
            _isConnecting = true;
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
            _isConnected = true;
            _isConnecting = false;
        }
    }
};
