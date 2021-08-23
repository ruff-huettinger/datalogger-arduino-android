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
    public int currentRunningInterval { get; set; } = 0;
    public Dictionary<ID, float> sensorValues { get; set; } = new Dictionary<ID, float>();
    public DateTime lastRSSITime { get; set; } = new DateTime();

    /// <summary>
    /// The updated time table
    /// </summary>
    public MODEOFHOUR[] hourModes { get; set; } = new MODEOFHOUR[24];

    /// <summary>
    /// The timetable currently running on sensor-egg (at the time of BLE-start on egg) 
    /// </summary>
    public MODEOFHOUR[] currentRunningMode { get; set; } = new MODEOFHOUR[24];

    /// <summary>
    /// The timetable designed by user and saved for selecting in dropdown menu
    /// </summary>
    public MODEOFHOUR[] customMode { get; set; } = new MODEOFHOUR[24];

    /// <summary>
    /// The default BLE-hours for the table-presets
    /// <br> - must also be changed in UI class </br>
    /// </summary>
    byte[] defBLEHour = { 10, 15 };

    void Start()
    {
    }

    void Update()
    {
    }

    /// <summary>
    /// Takes the byte-array from Egg's sensor-characteristic and writes it to Dictionary with:
    /// <br> - Sensor-ID </br>
    /// <br> - Sensor-Value </br>
    /// <br> Later, the UI uses the ID to display the Value in the according UI-element </br>
    /// </summary>
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

    /// <summary>
    /// Updates the current hourModes when a Preset is selected in the dropdown menu
    /// </summary>
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
