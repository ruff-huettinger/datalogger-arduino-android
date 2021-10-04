using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EggBLE : MonoBehaviour
{
    // The following variables store information about the ble-service and characteristics
    // They must be synchronized with Arduino-Code
    private const string DeviceName = "EggCallback";
    private const string serviceUUID = "f1111a7f-0000-41f9-a127-aedf35a799b3";
    private const string timeUUID = "f1111a7f-0001-41f9-a127-aedf35a799b3";
    private const string modesUUID = "f1111a7f-0002-41f9-a127-aedf35a799b3";
    private const string intervalUUID = "f1111a7f-0003-41f9-a127-aedf35a799b3";
    private const string batterySpaceUUID = "f1111a7f-0004-41f9-a127-aedf35a799b3";
    private const string sensorsUUID = "f1111a7f-0005-41f9-a127-aedf35a799b3";
    private const string updateUUID = "f1111a7f-0006-41f9-a127-aedf35a799b3";
    private const string startedUUID = "f1111a7f-0007-41f9-a127-aedf35a799b3";

    private string _deviceAddress;

    public EggState state;
    public ElectronicEgg egg;


    // Start is called before the first frame update
    void Start()
    {
        this.Invoke(() =>
        {
            BluetoothLEHardwareInterface.Initialize(true, false, () =>
            {
                ElectronicEgg.PrintLog("Initialized BLE Interface");
                egg.currentState = APPSTATES.DISCONNECTED;
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

    }

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
                //ElectronicEgg.PrintError("Update RSSI: " + rssi);
                state.rssi = rssi;
                state.lastRSSITime = DateTime.Now;
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
                //this.Invoke(() => { egg.currentState = APPSTATES.CONNECTED; }, 1.0f);
            }, null, null, (x) =>
            {
                egg.currentState = APPSTATES.DISCONNECTED;
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
            this.Invoke(Subscribe, 1.0f);
        });
    }


    /// <summary>
    /// Subscribe to notify-chracteristics of the egg service (e.g. Sensors-Characteristic)
    /// </summary>
    void Subscribe()
    {
        BluetoothLEHardwareInterface.SubscribeCharacteristic(_deviceAddress, serviceUUID, sensorsUUID, (notifyCharacteristic) =>
        {
            // Read all chracteristics after subscribing is completed
            this.Invoke(() => Read(startedUUID), 0.0f);
            this.Invoke(() => Read(batterySpaceUUID), 0.2f);
            this.Invoke(() => Read(modesUUID), 0.4f);
            this.Invoke(() => Read(intervalUUID), 0.6f);
            this.Invoke(RefreshSensors, 0.8f);
            this.Invoke(() => { egg.currentState = APPSTATES.CONNECTED; }, 1.0f);
            this.Invoke(() => SendTime(), 2.0f);

        }, (characteristicUUID, bytes) =>
        {
            // cb for notifications of changed sensor values
            ElectronicEgg.PrintLog("NumOfBytes: " + bytes.Length.ToString("0"));
            ElectronicEgg.PrintLog("Time: " + System.BitConverter.ToUInt32(bytes, 0).ToString());
            state.SetSensorInfos(bytes);
            egg.OnNewSensorValues();
        });
    }

    /// <summary>
    /// Disconnect from a device using the deviceAdress
    /// </summary>
    public void Disconnect()
    {
        BluetoothLEHardwareInterface.DisconnectPeripheral(_deviceAddress, (test) =>
        {
            egg.currentState = APPSTATES.DISCONNECTED;
        });
    }

    void SendTime()
    {
        System.DateTime epochStart = new System.DateTime(1970, 1, 1, 0, 0, 0, System.DateTimeKind.Utc);
        uint cur_time = (uint)(System.DateTime.Now - epochStart).TotalSeconds;
        ElectronicEgg.PrintLog("Sending Time:" + cur_time.ToString());
        byte[] x = BitConverter.GetBytes(cur_time);
        Write(timeUUID, x);
    }

    public void SendModes(MODEOFHOUR[] hourModes)
    {
        // Send the modes-data in 12 bytes instead of 24 to keep the arduinos mtu

        byte[] data = new byte[12];
        for (int i = 0; i < 24; i = i + 2)
        {
            byte firstValue = (byte)state.hourModes[i];
            byte secondValue = (byte)state.hourModes[i + 1];
            data[i / 2] = (byte)(firstValue + secondValue * 3);
        }
        Write(modesUUID, data);
    }

    public void SendInterval(int interval)
    {
        byte[] myByte = { (byte)interval };
        this.Invoke(() => Write(intervalUUID, myByte), 1.0f);
    }

    public void SendStartCommand()
    {
        Write(startedUUID, BitConverter.GetBytes(1));
    }

    public void SendEndCommand()
    {
        Write(startedUUID, BitConverter.GetBytes(2));
    }


    public void RefreshSensors()
    {
        byte[] data = { (byte)0x01 };
        Write(updateUUID, data);
    }

    /// <summary>
    /// A specific characterisitcs is read and its value is stored in EggState
    /// </summary>
    void Read(string uuid)
    {
        ElectronicEgg.PrintLog("Reading: " + uuid);

        BluetoothLEHardwareInterface.ReadCharacteristic(_deviceAddress, serviceUUID, uuid, (characteristic, bytes) =>
        {
            if (uuid.Equals(batterySpaceUUID))
            {
                ElectronicEgg.PrintLog("Bat Value: " + (System.BitConverter.ToSingle(bytes, 0)).ToString("0.00"));
                ElectronicEgg.PrintLog("SDSize Value: " + (System.BitConverter.ToUInt32(bytes, 4)).ToString("0.00"));
                ElectronicEgg.PrintLog("WrittenBytes Value: " + (System.BitConverter.ToUInt32(bytes, 8)).ToString("0.00"));

                state.batteryValue = System.BitConverter.ToSingle(bytes, 0);
                state.sdTotalSize = System.BitConverter.ToUInt32(bytes, 4) * 1000 * 1000; // mb -> bytes
                state.sdWrittenBytes = System.BitConverter.ToUInt32(bytes, 8);
                state.sdFillPercentage = state.sdWrittenBytes / state.sdTotalSize;
            }

            else if (uuid.Equals(startedUUID))
            {
                if (bytes[0] == 0)
                {
                    state.started = false;
                    state.sdInitialized = true;
                    state.genuineSD = true;
                }
                else if (bytes[0] == 1)
                {
                    state.started = true;
                    state.sdInitialized = true;
                    state.genuineSD = true;
                }
                else if (bytes[0] == 2)
                {
                    state.started = false;
                    state.sdInitialized = false;
                    state.genuineSD = true;
                }

                else if (bytes[0] == 3)
                {
                    state.started = false;
                    state.sdInitialized = true;
                    state.genuineSD = false;
                }
            }

            else if (uuid.Equals(modesUUID))
            {
                byte[] unzip = bleToMode(bytes);
                for (int i = 0; i < 24; i++)
                {
                    state.hourModes[i] = (MODEOFHOUR)unzip[i];
                }
                state.hourModes.CopyTo(state.currentRunningModes, 0);
                state.hourModes.CopyTo(state.customModes, 0);
            }
            else if (uuid.Equals(intervalUUID))
            {
                ElectronicEgg.PrintLog("interval :" + bytes[0].ToString());
                state.interval = (int)bytes[0];
                state.currentRunningInterval = (int)bytes[0];
            }
        });
    }

    /// <summary>
    /// Writes an array of bytes to a specific characteristic
    /// </summary>
    public void Write(string uuid, byte[] value)
    {
        BluetoothLEHardwareInterface.WriteCharacteristic(_deviceAddress, serviceUUID, uuid, value, value.Length, true, (characteristicUUID) =>
        {
        });
    }

    byte[] bleToMode(byte[] small)
    {
        byte[] large = new byte[24];
        for (int i = 0; i < 12; i++)
        {
            byte fullNum = small[i];
            byte firstValue = (byte)(fullNum % 3);
            byte secondValue = (byte)((fullNum / 3) % 3);
            large[i * 2] = firstValue;
            large[i * 2 + 1] = secondValue;
        }
        return large;
    }

    byte[] modeToBLE(byte[] large)
    {
        byte[] small = new byte[24];
        for (int i = 0; i < 24; i++)
        {
            byte firstValue = (byte)large[i];
            byte secondValue = (byte)large[i + 1];
            small[i / 2] = (byte)(firstValue + secondValue * 3);
        }
        return small;
    }
}
