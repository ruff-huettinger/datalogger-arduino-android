using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;
using System.Collections.Generic;

public class ElectronicEgg : MonoBehaviour
{
    // toDo: What exactly happens on cancel button click?

    public EggBLE ble;
    public EggUI ui;
    public EggState state;

    public APPSTATES currentState { get; set; } = APPSTATES.STARTED;

    APPSTATES previousState = APPSTATES.STARTED;

    //----Functions

    void Start()
    {
        PrintLog("Start");
        ui.SwitchScreenTo("main");

        // sets the app to fullscreen with status-bar but without navigation bar
        ApplicationChrome.SetupAndroidTheme(ApplicationChrome.ToARGB(Color.black), ApplicationChrome.ToARGB(Color.black));
    }

    void Update()
    {
        //currentState = APPSTATES.CONNECTED;
        //APPSTATES nextState = currentState;

        switch (currentState)
        {
            case APPSTATES.DISCONNECTED:
                if (currentState != previousState)
                {
                    ble.StartRSSIScan();
                    ui.SetStatusIcon("Getrennt");
                    ui.ChangeStatusButton(true, "Verbinden");
                    ui.HidePanel(ui.functionsPanel);
                    ui.ShowSensorOverlay(true);
                    ui.HideButton(ui.refreshBtn);
                    ui.ShowPanel(ui.rssiPanel);
                }
                ui.UpdateRSSI(state.rssi, state.lastRSSITime);
                break;

            case APPSTATES.CONNECTING:
                if (currentState != previousState)
                {
                    ble.StopRSSIScan();
                    this.Invoke(ble.StartConnection, 0.5f);
                    ui.ChangeStatusButton(false, "Verbinde...");
                    ui.SetStatusIcon("Verbinde");
                    ui.HidePanel(ui.rssiPanel);
                }
                break;
            case APPSTATES.CONNECTED:
                if (currentState != previousState)
                {
                    ui.SetStatusIcon("Verbunden");
                    ui.ChangeStatusButton(true, "Trennen");
                    ui.ShowPanel(ui.functionsPanel);
                    ui.ShowSensorOverlay(false);
                    ui.ShowButton(ui.refreshBtn);
                    ui.ShowSensorValues(state.sensorValues);
                    ui.UpdateSliders(state.batteryValue, state.sdFillPercentage);
                    ui.SetStartButton(state.started);

                    if (!ui.togglesCreated)
                    {
                        ui.CreateToggles();
                        AddListenersToToggles();
                    }
                    ui.UpdateToggles(state.hourModes, state.currentRunningMode);
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
        //currentState = nextState;
    }

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

    bool toggleAutoRefresh = false;
    public void OnAutoRefreshButton()
    {
        if (!toggleAutoRefresh)
        {
            InvokeRepeating("OnRefreshButton", 0, 0.5F);
            toggleAutoRefresh = true;
        }
        else
        {
            CancelInvoke("OnRefreshButton");
            toggleAutoRefresh = false;
        }
    }

    public void OnNewSensorValues()
    {
        ui.ShowSensorValues(state.sensorValues);
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

    public void OnModesDropChanged(Dropdown dropDown)
    {
        ElectronicEgg.PrintLog("MODES CHANGED -> " + dropDown.value);
        state.UpdateModes(dropDown.value);
        ui.UpdateToggles(state.hourModes, state.currentRunningMode);
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


    public void OnSubmitButton()
    {
        ElectronicEgg.PrintLog("Pressed submit btn");

        ui.SetButtonEnabled(ui.submitBtn, false);
        /*
        this.Invoke(() =>
        {
        }, 3.0f);
        */
        state.hourModes.CopyTo(state.currentRunningMode, 0);

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

    public void AddListenersToToggles()
    {
        for (int i = 0; i < 24; i++)
        {
            int z = i;
            ui.modeToggles[z].onValueChanged.AddListener(delegate
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
                state.hourModes.CopyTo(state.customMode, 0);
                ui.UpdateToggles(state.hourModes, state.currentRunningMode);
            });
        }
    }


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