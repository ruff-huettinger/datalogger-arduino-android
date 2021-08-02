using System.Collections;
using System.Collections.Generic;
using UnityEngine;

/// <summary>
/// Contains the IDs of all possible sensors of the egg 
/// <para> Used to assign certain values to certain ui-elements</para>
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
/// <para> Same order as in Arduino code (off, audio, ble)</para>
/// </summary>
public enum MODEOFHOUR
{
    OFF,
    AUDIO,
    BLE
};

enum TABLE
{
    SENSORONLY,
    AUDIOONLY,
    USERGEN
};

public enum APPSTATES
{
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    DISCONNECTING,
    STARTED
};