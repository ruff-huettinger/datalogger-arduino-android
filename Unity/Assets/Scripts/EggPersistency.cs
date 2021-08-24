using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using UnityEngine;

/// <summary>
/// A class for persistent data management using the smartphone's storage
/// </summary>
public class EggPersistency : MonoBehaviour
{
    /// <summary>
    /// Saves a timetable to PlayerPrefs
    /// <br> It can be loaded after the app was closed </br>
    /// </summary>
    public void SaveTimetable(MODEOFHOUR[] table)
    {
        for (int i = 0; i < 24; i++)
        {
            PlayerPrefs.SetInt(i.ToString(), (int)table[i]);
        }
    }

    /// <summary>
    /// Loads a timetable from the PlayerPrefs (persistent smartphone storage)
    /// </summary>
    private MODEOFHOUR[] LoadTimetable()
    {
        MODEOFHOUR[] lastSavedTable = new MODEOFHOUR[24];
        for (int i = 0; i < 24; i++)
        {
            lastSavedTable[i] = (MODEOFHOUR)PlayerPrefs.GetInt(i.ToString());
        }
        return lastSavedTable;
    }

    /// <summary>
    /// Calculates the timedef until the next BLE-hour
    /// <br> A simple algorithm and the Unity-classes "DateTime" and "TimeSpan" are used </br>
    /// </summary>
    public TimeSpan GetBLETimeDif()
    {
        MODEOFHOUR[] lastSavedTable = LoadTimetable();
        int nextHour = DateTime.Now.Hour + 1;
        int difHours = 0;

        for (int i = nextHour; i < nextHour + 24; i++)
        {
            difHours++;
            int index = i % 24;
            if (lastSavedTable[index] == MODEOFHOUR.BLE)
            {
                break;
            }
        }

        DateTime nextBLETime = DateTime.Now.AddHours(difHours);
        nextBLETime = nextBLETime.Subtract(new TimeSpan(0, DateTime.Now.Minute, DateTime.Now.Second - 1));

        // This avoids the problem of negative Timespans in the case of nextTime = now
        if (nextBLETime < DateTime.Now)
        {
            nextBLETime = nextBLETime.AddDays(1);
        }
        return nextBLETime - DateTime.Now;
    }
}

