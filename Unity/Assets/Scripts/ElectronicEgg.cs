/*
	Electronic Egg Unity-app

	Created 23 08 2021
	By Johannes Brunner
*/

using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;
using System.Collections.Generic;

public class ElectronicEgg : MonoBehaviour
{
    // toDo: Minor Adjustments to dropdowns-ui
    // toDo: Increase toggles' size
    // toDo: Update ui.NextBLETimeText to new Screendesign

    public EggBLE ble;
    public EggUI ui;
    public EggState state;
    public EggPersistency pers;

    public APPSTATES currentState { get; set; } = APPSTATES.STARTED;

    APPSTATES previousState = APPSTATES.STARTED;

    private float nextCountdownTime = 0;

    //----Functions

    void Start()
    {
        PrintLog("Start");
        ui.SwitchScreenTo("main");

        // sets the app to fullscreen with status-bar but without navigation bar
        Utility.SetupAndroidTheme(Utility.ToARGB(Color.black), Utility.ToARGB(Color.black));

        // enable Bluetooth Adapter on App start
        Utility.setAndroidBluetoothEnabled();
    }

    // --- State machine ---

    /// <summary>
    /// The main method of the electronic egg app:
    /// <br> - A state machine is used to trigger functions of BLE and UI </br>
    /// <br> - The different states are selected by button-clicks and by BLE events </br>
    /// </summary>
    void Update()
    {
        // uncomment for simulating connected state
        //currentState = APPSTATES.CONNECTED;

        switch (currentState)
        {
            case APPSTATES.DISCONNECTED:
                if (currentState != previousState)
                {
                    ble.StartRSSIScan();
                    ui.UpdateDisconnected();
                    if (previousState != APPSTATES.STARTED)
                    {
                        pers.SaveTimetable(state.currentRunningModes);
                    }
                }
                ui.UpdateRSSI(state.rssi, state.lastRSSITime);

                if (Time.time >= nextCountdownTime)
                {
                    ui.UpdateBLECountdown(pers.GetBLETimeDif());
                    nextCountdownTime = Time.time + 1.0f;
                }
                break;

            case APPSTATES.CONNECTING:
                if (currentState != previousState)
                {
                    ble.StopRSSIScan();
                    this.Invoke(ble.StartConnection, 0.5f);
                    ui.UpdateConnecting();
                }
                break;

            case APPSTATES.CONNECTED:
                if (currentState != previousState)
                {
                    ui.UpdatedConnected(state.sensorValues, state.batteryValue, state.sdFillPercentage, state.started, state.sdWrittenBytes, state.sdInitialized, state.genuineSD);

                    if (!ui.togglesCreated)
                    {
                        ui.CreateToggles();
                        AddListenersToToggles();
                    }
                    ui.UpdateToggles(state.hourModes, state.currentRunningModes);
                    ui.SetIntervalDropdown(state.interval);
                }
                break;

            case APPSTATES.DISCONNECTING:
                if (currentState != previousState)
                {
                    ble.Disconnect();
                    ui.ChangeStatusButton(true, "Trenne...");
                }
                break;

            default:
                break;
        }
        previousState = currentState;
    }

    // --- Listeners ---

    public void OnConnectButton()
    {
        if (currentState == APPSTATES.DISCONNECTED || currentState == APPSTATES.DISCONNECTED)
        {
            currentState = APPSTATES.CONNECTING;
        }
        else
        {
            currentState = APPSTATES.DISCONNECTING;
        }
    }

    public void OnRefreshButton()
    {
        ble.RefreshSensors();
        ui.SetButtonEnabled(ui.refreshBtn, false);
        this.Invoke(() => { ui.SetButtonEnabled(ui.refreshBtn, true); }, 1.0f);
    }

    public void OnSettingsButton()
    {
        ElectronicEgg.PrintLog("Pressed Settings Button");
        ui.SwitchScreenTo("settings");
    }

    public void OnBackButton()
    {
        ui.SwitchScreenTo("main");
    }

    public void OnSubmitButton()
    {
        ElectronicEgg.PrintLog("Pressed submit btn");

        ui.SetButtonPressed(ui.submitBtn, true);

        this.Invoke(() =>
        {
            ui.SetButtonPressed(ui.submitBtn, false);
            ui.SetButtonEnabled(ui.submitBtn, false);
        }, 2.0f);

        state.hourModes.CopyTo(state.currentRunningModes, 0);

        // Send the modes table

        ble.SendModes(state.hourModes);

        // Send the interval value

        List<Dropdown.OptionData> menuOptions = ui.dropInterval.GetComponent<Dropdown>().options;
        string value = menuOptions[ui.dropInterval.value].text;
        int myInt = int.Parse(value);
        ble.SendInterval(myInt);
    }

    public void OnStartButton()
    {
        //PrintError(ui.startedBtn.GetComponentInChildren<Text>().text);
        if (ui.startedBtn.GetComponentInChildren<Text>().text == "Beenden")
        {
            ui.OpenPopup(ui.cancelWarningPanel);
        }
        else
        {
            ui.OpenPopup(ui.formatWarningPanel);
        }
    }

    public void OnPopupCancelButton()
    {
        ui.ClosePopup(ui.formatWarningPanel);
        ui.ClosePopup(ui.cancelWarningPanel);
    }

    public void OnPopupStartButton()
    {
        ui.ClosePopup(ui.formatWarningPanel);
        ble.SendStartCommand();
        currentState = APPSTATES.DISCONNECTING;
    }

    public void OnPopupEndButton()
    {
        ui.ClosePopup(ui.cancelWarningPanel);
        ble.SendEndCommand();
        currentState = APPSTATES.DISCONNECTING;
    }

    public void OnNewSensorValues()
    {
        ui.ShowSensorValues(state.sensorValues);
    }

    public void OnModesDropChanged(Dropdown dropDown)
    {
        ElectronicEgg.PrintLog("MODES CHANGED -> " + dropDown.value);
        state.UpdateModes(dropDown.value);
        ui.UpdateToggles(state.hourModes, state.currentRunningModes);
    }

    public void OnIntervalDropChanged(Dropdown dropDown)
    {
        ElectronicEgg.PrintLog("INTERVAL CHANGED -> " + dropDown.value);
        int selectedInterval = int.Parse(dropDown.options[dropDown.value].text);
        if (selectedInterval != state.currentRunningInterval)
        {
            ui.SetButtonEnabled(ui.submitBtn, true);
        }
    }

    public void AddListenersToToggles()
    {
        for (int i = 0; i < 24; i++)
        {
            int z = i;
            ui.hourToggles[z].onValueChanged.AddListener(delegate
            {
                if (ui.selectionToggles[0].isOn)
                {
                    if (state.hourModes[z] != MODEOFHOUR.AUDIO)
                    {
                        state.hourModes[z] = MODEOFHOUR.AUDIO;
                    }
                    else
                    {
                        state.hourModes[z] = MODEOFHOUR.OFF;
                    }
                }
                else if (ui.selectionToggles[1].isOn)
                {
                    if (state.hourModes[z] != MODEOFHOUR.BLE)
                    {
                        state.hourModes[z] = MODEOFHOUR.BLE;
                    }
                    else
                    {
                        state.hourModes[z] = MODEOFHOUR.OFF;
                    }
                }
                else
                {
                    state.hourModes[z] = MODEOFHOUR.OFF;
                }
                state.hourModes.CopyTo(state.customModes, 0);
                ui.UpdateToggles(state.hourModes, state.currentRunningModes);
            });
        }
    }

    // --- Debug Prints to android logCat ---

#if DEBUG
    static long logCnt = 0;
#endif
    public static void PrintLog(string x)
    {
#if DEBUG
        Debug.LogWarning(x + " [" + "Log:" + System.Convert.ToString(logCnt) + "]");
        //Console.WriteLine((x + " [" + "Log:" + System.Convert.ToString(logCnt) + "]"));
        logCnt++;
#else
#endif
    }
    public static void PrintError(string x)
    {
#if DEBUG
        Debug.LogError(x + " [" + "Log:" + System.Convert.ToString(logCnt) + "]");
        //Console.WriteLine((x + " [" + "Log:" + System.Convert.ToString(logCnt) + "]"));
        logCnt++;
#else
#endif
    }
}