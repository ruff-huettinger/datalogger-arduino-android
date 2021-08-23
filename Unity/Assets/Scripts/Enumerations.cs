using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Contains the IDs of all possible sensors of the egg 
/// <br> - Used to assign certain values to certain ui-elements</br>
/// </summary>
public enum ID
{
    ACCX,
    ACCY,
    ACCZ,
    MAGX,
    MAGY,
    MAGZ,
    TEMP,
    HUMI,
    LIGHT,
    LIGHTANAONE,
    LIGHTANATWO,
    BATSENS,
};

/// <summary>
/// Contains the three possible hour-modes 
/// <br> Same order as in Arduino code (off, audio, ble)</br>
/// </summary>
public enum MODEOFHOUR
{
    OFF,
    AUDIO,
    BLE
};

/// <summary>
/// The three presets:
/// <br> - The first two are also defined in Arduino code </br>
/// </summary>
enum TABLE
{
    SENSORONLY,
    AUDIOONLY,
    USERGEN
};

/// <summary>
/// The different states of the App:
/// <br> - UI is updated due to the active state </br>
/// <br> - BLE actions are triggered by a selected state </br>
/// <br> - BLE events can cause a change of state </br>
/// </summary>
public enum APPSTATES
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
    STARTED
};