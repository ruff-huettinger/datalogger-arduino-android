
/*
using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;

public class EggFunctionsTest : MonoBehaviour
{
    public string DeviceName = "ledbtnFunc";
    public string ServiceUUID = "A9E90000-194C-4523-A473-5FDF36AA4D20";
    public string LedUUID = "A9E90001-194C-4523-A473-5FDF36AA4D20";
    public string writeUUID = "A9E90002-194C-4523-A473-5FDF36AA4D20";
    public string readUUID = "A9E90003-194C-4523-A473-5FDF36AA4D20";


    private bool _connected = false;
    private bool _connecting = false;
    private bool _disconnecting = false;
    private string _deviceAddress;


    public GameObject btnDConnect;
    public GameObject btnConnect;

    public Text StatusText;

    private bool ledON = false;


    //----Functions

    // Use this for initialization
    void Start()
    {
        Reset();
        SetStatusText("Ready to connect");
    }

    void Update()
    {
    }


    void Reset()
    {
        _connected = false;
        _deviceAddress = null;
    }

    void InitBLE()
    {
        BluetoothLEHardwareInterface.Initialize(true, false, () =>
        {
            StartScanning();
        }, (error) =>
        {
            SetStatusText("Error during initialize: " + error);
        });
    }

    void StartScanning()
    {
        SetStatusText("Scanning for " + DeviceName);

        BluetoothLEHardwareInterface.ScanForPeripheralsWithServices(null, (address, name) =>
        {
            if (name.Contains(DeviceName))
            {
                SetStatusText("Found " + DeviceName);


                BluetoothLEHardwareInterface.StopScan();

                // found a device with the name we want
                // this example does not deal with finding more than one
                _deviceAddress = address;
                Connect();
            }

        }, (address, name, rssi, bytes) =>
        {

            // use this one if the device responses with manufacturer specific data and the rssi
            if (name.Contains(DeviceName))
            {
                SetStatusText("Found " + DeviceName);
                BluetoothLEHardwareInterface.StopScan();

                // found a device with the name we want
                // this example does not deal with finding more than one
                _deviceAddress = address;
                Connect();
            }

        }, false);
    }

    void Connect()
    {
        BluetoothLEHardwareInterface.ConnectToPeripheral(_deviceAddress, (serviceUUID) =>
        {
            SetStatusText("Connected");
            _connected = true;
            _connecting = false;
            this.Invoke(RequestMTU, 1.0f);
        }, null, null);
    }


    void RequestMTU()
    {
        BluetoothLEHardwareInterface.RequestMtu(_deviceAddress, 60, (address, newMTU) =>
                            {
                                SetStatusText("MTU set to " + newMTU.ToString());
                                this.Invoke(Subscribe, 1.0f);
                            });
    }

    void Subscribe()
    {
        PrintDebug("Subscribing now");
        PrintDebug(readUUID);


        BluetoothLEHardwareInterface.SubscribeCharacteristic(_deviceAddress, ServiceUUID, "A9E90003-194C-4523-A473-5FDF36AA4D20", (notifyCharacteristic) =>
        {
            // cb for succesfull subscribing
            PrintDebug("Subscribed");

            SendTime();

        }, (characteristicUUID, bytes) =>
        {
            // cb for notifications
            PrintDebug("NumOfBytes: " + bytes.Length.ToString("0"));

            PrintDebug("Time: " + System.BitConverter.ToUInt32(bytes, 0).ToString());

            for (int i = 0; i < bytes.Length / 5; i++)
            {
                PrintDebug("Sensor ID: " + bytes[i * 5 + 4].ToString()); ;
                PrintDebug("Sensor Value: " + (System.BitConverter.ToSingle(bytes, (i * 5 + 1 + 4))).ToString("0.00"));
            }
        });

    }

    void Disconnect()
    {
        BluetoothLEHardwareInterface.DisconnectPeripheral(_deviceAddress, null);

        BluetoothLEHardwareInterface.DeInitialize(() =>
        {
            SetStatusText("Disconnected");
            _connected = false;
            _disconnecting = false;
        }
        );
    }

    void SendTime()
    {
        System.DateTime epochStart = new System.DateTime(1970, 1, 1, 0, 0, 0, System.DateTimeKind.Utc);
        int cur_time = (int)(System.DateTime.UtcNow - epochStart).TotalSeconds;
        PrintDebug(cur_time.ToString());

        // toDo: implement
    }

    //-------------Callbacks

    public void OnCButton()
    {
        PrintDebug("Pressed C Button");
        if (!_connected && !_connecting)
        {
            _connecting = true;
            InitBLE();
        }
    }

    public void OnDCButton()
    {
        PrintDebug("Pressed DC Button");
        if (_connected && !_disconnecting)
        {
            _disconnecting = true;
            Disconnect();
        }
    }


    public void OnToggleButton()
    {
        PrintDebug("Pressed Toggle Button");

        ledON = !ledON;
        if (ledON)
        {
            SendByte((byte)0x01);
        }
        else
        {
            SendByte((byte)0x00);
        }
    }

    public void OnReadButton()
    {

        PrintDebug("Pressed Read Button");

        BluetoothLEHardwareInterface.ReadCharacteristic(_deviceAddress, ServiceUUID, readUUID, (characteristic, bytes) =>
        {
            //PrintDebug("test");
            PrintDebug(System.Convert.ToString(bytes[0]));
        });
    }

    public void onRefreshButton()
    {
        byte[] data = { (byte)0x00 };
        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, writeUUID, data, data.Length, true, (characteristicUUID) =>
        {
        });
    }


    //------------Helper

    private void SetStatusText(string value)
    {
        StatusText.text = value;
    }



    string FullUUID(string uuid)
    {
        string fullUUID = uuid;
        if (fullUUID.Length == 4)
            fullUUID = "0000" + uuid + "-0000-1000-8000-00805f9b34fb";

        return fullUUID;
    }

    bool IsEqual(string uuid1, string uuid2)
    {
        if (uuid1.Length == 4)
            uuid1 = FullUUID(uuid1);
        if (uuid2.Length == 4)
            uuid2 = FullUUID(uuid2);

        return (uuid1.ToUpper().Equals(uuid2.ToUpper()));
    }

    void SendByte(byte value)
    {
        byte[] data = { value };
        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, ServiceUUID, LedUUID, data, data.Length, true, (characteristicUUID) =>
        {
        });
    }

    public static uint SwapEndianness(uint value)
    {
        var b1 = (value >> 0) & 0xff;
        var b2 = (value >> 8) & 0xff;
        var b3 = (value >> 16) & 0xff;
        var b4 = (value >> 24) & 0xff;

        return b1 << 24 | b2 << 16 | b3 << 8 | b4 << 0;
    }


#if DEBUG
long logCnt = 0;
#endif

    void PrintDebug(string x)
    {
#if DEBUG
    Debug.LogWarning(x + " [" + "Log:" + System.Convert.ToString(logCnt) +  "]");
    logCnt++;
#else

#endif
    }
}

/*

public static class Utility
{
    public static void Invoke(this MonoBehaviour mb, Action f, float delay)
    {
        mb.StartCoroutine(InvokeRoutine(f, delay));
    }

    private static IEnumerator InvokeRoutine(System.Action f, float delay)
    {
        yield return new WaitForSeconds(delay);
        f();
    }
}
*/

