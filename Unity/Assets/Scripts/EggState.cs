using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class EggState : MonoBehaviour
{
    public float rssi { get; set; } = 0.0f;
    public uint sdTotalSize { get; set; } = 0;
    public uint sdWrittenBytes { get; set; } = 0;
    public float batteryValue { get; set; } = 0.0f;
    public float sdFillPercentage { get; set; } = 0.0f;
    public bool started { get; set; } = false;
    public int interval { get; set; } = 0;
    public MODEOFHOUR[] hourModes { get; set; } = new MODEOFHOUR[24];
    public MODEOFHOUR[] customMode { get; set; } = new MODEOFHOUR[24];

    public Dictionary<ID, float> sensorValues { get; set; } = new Dictionary<ID, float>();
    public bool present { get; set; } = false;

    public DateTime lastRSSITime { get; set; } = new DateTime();


    /// <summary>
    /// The default BLE-hours for the table-presets
    /// </summary>
    byte[] defBLEHour = { 10, 15 };


    void Start()
    {
    }

    // Update is called once per frame
    void Update()
    {

    }


    public void SetSensorInfos(byte[] bytes)
    {
        for (int i = 0; i < bytes.Length / 5; i++)
        {
            ElectronicEgg.PrintLog("Sensor ID: " + bytes[i * 5 + 4].ToString());
            ElectronicEgg.PrintLog("Sensor Value: " + (System.BitConverter.ToSingle(bytes, (i * 5 + 1 + 4))).ToString("0.00"));

            int id = bytes[i * 5 + 4];
            float value = System.BitConverter.ToSingle(bytes, (i * 5 + 1 + 4));

            sensorValues[(ID)id] = value;
        }
    }

    public void UpdateModes(int dropValue)
    {
        switch (dropValue)
        {
            case (int)TABLE.SENSORONLY:
                for (int i = 0; i < hourModes.Length; i++)
                {
                    hourModes[i] = MODEOFHOUR.OFF;
                }
                hourModes[defBLEHour[0]] = MODEOFHOUR.BLE;
                hourModes[defBLEHour[1]] = MODEOFHOUR.BLE;
                break;

            case (int)TABLE.AUDIOONLY:
                for (int i = 0; i < hourModes.Length; i++)
                {
                    hourModes[i] = MODEOFHOUR.AUDIO;
                }
                hourModes[defBLEHour[0]] = MODEOFHOUR.BLE;
                hourModes[defBLEHour[1]] = MODEOFHOUR.BLE;
                break;

            case (int)TABLE.USERGEN:

                for (int i = 0; i < hourModes.Length; i++)
                {
                    hourModes[i] = customMode[i];
                }
                break;

            default:
                ElectronicEgg.PrintLog("Default case");
                break;
        }
    }
}
