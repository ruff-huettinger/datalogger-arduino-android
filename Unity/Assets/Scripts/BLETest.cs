using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Linq;

public class BLETest : MonoBehaviour
{
    private const string DeviceName = "EI_TX8_M1";

    bool connectedState = false;
    bool connecting = false;
    public Button btnConnect;
    public Button btnDisconnect;
    int rssiVal = 0;
    public Text rssiText;
    public Text connectedText;
    private string _deviceAddress;
    DateTime lastRSSITime;


    // Start is called before the first frame update
    void Start()
    {
        this.Invoke(() =>
        {
            BluetoothLEHardwareInterface.Initialize(true, false, () =>
            {
                ElectronicEgg.PrintLog("Initialized BLE Interface");
                connectedState = false;
                StartRSSIScan();
            }, (error) =>
            {
                ElectronicEgg.PrintError("BLE Init Error");
            });
        }
        , 0.1f);
    }

    // Update is called once per frame
    void Update()
    {
        if (!connectedState)
        {
            if (!connecting)
            {
                TimeSpan difference = DateTime.Now.Subtract(lastRSSITime);
                if (difference.Seconds < 5)
                {
                    SetButtonEnabled(btnConnect, true);
                    connectedText.text = "Disconnected | " + DeviceName + " gefunden";
                }
                else
                {
                    SetButtonEnabled(btnConnect, false);
                    connectedText.text = "Disconnected | Kein " + DeviceName + " gefunden";
                }
                SetButtonEnabled(btnDisconnect, false);
                rssiText.text = "RSSI: " + rssiVal + "dBm";
            }
            else {
                connectedText.text = "Connecting...";
            }
        }
        else
        {
            SetButtonEnabled(btnConnect, false);
            SetButtonEnabled(btnDisconnect, true);
            connectedText.text = "Connected!";
        }

    }

    public void SetButtonEnabled(Button btn, bool value)
    {
        if (value)
        {
            btn.GetComponentInChildren<Text>().color = Color.black;
        }
        else
        {
            btn.GetComponentInChildren<Text>().color = Color.grey;
        }
        btn.interactable = value;
    }

    public void onBtnConnect()
    {
        ElectronicEgg.PrintLog("On btn conn");
        SetButtonEnabled(btnConnect, false);
        //lastRSSITime = lastRSSITime.AddSeconds(-5);
        connecting = true;
        StopRSSIScan();
        this.Invoke(StartConnection, 0.5f);
    }

    public void onBtnDisconnect()
    {
        ElectronicEgg.PrintLog("On btn disconn");
        Disconnect();
    }


    // BLE Stuff


    /// <summary>
    /// Perform RSSI-Scan when disconnected
    /// </summary>
    public void StartRSSIScan()
    {
        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, null, (address, name, rssi, bytes) =>
        {
            // use this one if the device responses with manufacturer specific data and the rssi
            // ElectronicEgg.PrintLog(name + rssi);
            if (name.Contains(DeviceName))
            {
                rssiVal = rssi;
                lastRSSITime = DateTime.Now;
            }
        }, true);
    }

    public void StopRSSIScan()
    {
        BluetoothLEHardwareInterface.StopScan();
    }

    public void StartConnection()
    {
        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) =>
        {
            if (name.Contains(DeviceName))
            {
                BluetoothLEHardwareInterface.StopScan();
                // found a device with the name we want
                // this example does not deal with finding more than one
                _deviceAddress = address;
                Connect();
            }

        }, null, false);
    }

    void Connect()
    {
        {
            BluetoothLEHardwareInterface.ConnectToPeripheral(_deviceAddress, (serviceUUID) =>
            {
                this.Invoke(RequestMTU, 1.0f);
            }, null, null, (x) =>
            {
                connectedState = false;
                connecting = false;
            });
        }
    }

    /// <summary>
    /// Request a higher MTU for transfering larger byte arrays
    /// </summary>
    void RequestMTU()
    {
        BluetoothLEHardwareInterface.RequestMtu(_deviceAddress, 100, (address, newMTU) =>
        {
            connectedState = true;
            connecting = false;
        });
    }


    /// <summary>
    /// Disconnect from a device using the deviceAdress
    /// </summary>
    public void Disconnect()
    {
        BluetoothLEHardwareInterface.DisconnectPeripheral(_deviceAddress, (test) =>
        {
            connectedState = false;
            connecting = false;
            StartRSSIScan();
        });
    }



}
