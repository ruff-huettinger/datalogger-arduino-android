using UnityEngine;
using UnityEngine.UI;
using System;
using System.Collections;
using System.Collections.Generic;

public class ElectronicEgg : MonoBehaviour
{
    // toDo: How to display RSSI:0?
    // toDo: Submit-Button active?
    // toDo: third Button in settingsPanel
    // toDo: What happens on cancel button click?

    public EggBLE ble;
    public EggUI ui;
    public EggState state;

    public APPSTATES currentState { get; set; } = APPSTATES.STARTED;

    APPSTATES previousState = APPSTATES.STARTED;

    //----Functions

    /// <summary>
    /// Init
    /// </summary>
    void Start()
    {
        PrintLog("Start");
        ui.SwitchScreenTo("main");
    }

    void Update()
    {
        //currentState = APPSTATES.CONNECTED;
        APPSTATES nextState = currentState;

        switch (currentState)
        {
            case APPSTATES.DISCONNECTED:
                if (currentState != previousState)
                {
                    ble.StartRSSIScan();
                    ui.SetStatusText("Getrennt");
                    ui.ChangeStatusButton(true, "Verbinden");
                    ui.HidePanel(ui.functionsPanel);
                    ui.SetPanelColor(ui.sensorPanel, new Color(0.5f, 0.5f, 0.5f));
                    ui.HideButton(ui.refreshBtn);
                    ui.SetStateText("Letzte Sensorwerte");
                    ui.ShowText(ui.RssiText);
                }
                ui.UpdateRSSI(state.rssi);
                break;

            case APPSTATES.CONNECTING:
                if (currentState != previousState)
                {
                    ble.StopRSSIScan();
                    ble.StartConnection();
                    ui.ChangeStatusButton(false, "Verbinde...");
                }
                break;
            case APPSTATES.CONNECTED:
                if (currentState != previousState)
                {
                    ui.SetStatusText("Verbunden");
                    ui.ChangeStatusButton(true, "Trennen");
                    ui.HideText(ui.RssiText);
                    ui.ShowPanel(ui.functionsPanel);
                    ui.SetPanelColor(ui.sensorPanel, new Color(0.9f, 0.9f, 0.9f));
                    ui.ShowButton(ui.refreshBtn);
                    ui.ShowSensorValues(state.sensorValues);
                    ui.SetStateText("Aktuelle Sensorwerte");
                    ui.UpdateSliders(state.batteryValue, state.sdFillPercentage);
                    ui.SetStartButton(state.started);
                    ui.HideText(ui.RssiText);
                    ui.UpdateCustomMode(state.hourModes);

                    if (!ui.togglesCreated)
                    {
                        ui.CreateToggles();
                        AddListenersToToggles();
                    }
                    ui.UpdateToggles(state.hourModes);
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
        currentState = nextState;
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

    public void OnDropdownSelect(Dropdown dropDown)
    {
        ElectronicEgg.PrintLog("DROP DOWN CHANGED -> " + dropDown.value);
        state.UpdateModes(dropDown.value);
        ui.UpdateToggles(state.hourModes);
    }


    public void OnSubmitButton()
    {
        ElectronicEgg.PrintLog("Pressed submit btn");

        ui.SetButtonColor(ui.submitBtn, Color.green);
        ui.SetButtonEnabled(ui.submitBtn, false);
        this.Invoke(() =>
        {
            ui.SetButtonEnabled(ui.submitBtn, true);
            ui.SetButtonColor(ui.submitBtn, Color.white);
        }, 3.0f);

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
        if (ui.startedBtn.GetComponentInChildren<Text>().text == "Abbrechen")
        {
            //ui.OpenPopup(ui.cancelWarningPanel);
        }
        else
        {
            ui.OpenPopup(ui.formatWarningPanel);
        }
    }

    public void OnPopupCancelButton()
    {
        ui.ClosePopup(ui.formatWarningPanel);
    }

    public void OnPopupStartButton()
    {
        ui.ClosePopup(ui.formatWarningPanel);
        ble.SendStartCommand();
        currentState = APPSTATES.DISCONNECTING;
    }

    public void AddListenersToToggles()
    {
        for (int i = 0; i < 24; i++)
        {
            int z = i;
            ui.modeToggles[z].onValueChanged.AddListener(delegate
            {
                /*
                if (state.hourModes[z] != MODEOFHOUR.OFF)
                {
                    state.hourModes[z] = MODEOFHOUR.OFF;
                }
                else*/
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
                ui.UpdateToggles(state.hourModes);
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