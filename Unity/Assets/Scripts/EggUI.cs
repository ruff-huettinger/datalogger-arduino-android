using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Linq;


public class EggUI : MonoBehaviour
{
    /// <summary>
    /// Timeout in secs for displaying the egg as unreachable
    /// </summary>
    private const double RSSI_TIMEOUT_S = 5.0f;
    private const int RSSI_MAX = 50;
    private const int RSSI_MIN = 95;

    public Button statusBtn;
    public Button refreshBtn;
    public Button startedBtn;
    public Button submitBtn;

    public Image connectedIcon;
    public Image disconnectedIcon;
    public Image connectingIcon;

    public Image sensorOverlay;

    /// <summary>
    /// 3d-representation of the acc-values
    /// </summary>
    public GameObject eggObj;
    public Quaternion eggOrigRotation;

    public Text StatusText;
    public Text SensorText;
    public Text TempText;
    public Text HumiText;
    public Text LightText;
    public Text BLEWarningText;
    public Text SDSpaceText;
    public Text BatValueText;
    public Text NextBLETimeText;
    public Text SDProblemWarningText;
    public Text rssiText;

    public Slider batSlider;
    public Slider spaceSlider;

    public GameObject mainScreen;
    public GameObject settingsScreen;
    public GameObject popupScreen;
    public Dictionary<string, GameObject> screenList = new Dictionary<string, GameObject>();

    public GameObject togglePanel;
    public GameObject statusPanel;
    public GameObject sensorPanel;
    public GameObject functionsPanel;
    public GameObject formatWarningPanel;
    public GameObject cancelWarningPanel;

    public GameObject rssiPanel;
    public Image rssiIcon;

    /// <summary>
    /// Dropdown for selecting the time-interval of sensor-measurings
    /// <br> Will be synchronized by the connected egg </br>
    /// </summary>
    public Dropdown dropInterval;

    /// <summary>
    /// Dropdown for selecting presets of timetables
    /// <br> Will be synchronized by the connected egg </br>
    /// <br> Will be overwritten by user if the toggles are changed </br>
    /// </summary>
    public Dropdown dropModes;

    /// <summary>
    /// Hour toggles displayed on settings screen
    /// </summary>
    public Toggle[] hourToggles { get; set; } = new Toggle[24];
    public GameObject togglePrefab;
    public GameObject txtPrefab;
    public bool togglesCreated { get; set; } = false;

    /// <summary>
    /// A toggle-group for selecting audio, ble of off
    /// <para> The selected value will be applied to the hourToggles OnToggleClick-event </para>
    /// </summary>
    public Toggle[] selectionToggles;

    /// <summary>
    /// The default BLE-hours for the table-presets
    /// <br> - must also be changed in state class </br>
    /// </summary>
    byte[] defBLEHour = { 10, 15 };

    private bool animatedIcon = false;
    private float nextAnimateTime = 0;

    void Awake()
    {
        screenList.Add("main", mainScreen);
        screenList.Add("settings", settingsScreen);
        screenList.Add("popup", popupScreen);
    }

    void Start()
    {
        eggOrigRotation = eggObj.transform.rotation;
    }

    // Update is called once per frame
    void Update()
    {
        if (animatedIcon && Time.time >= nextAnimateTime)
        {
            connectingIcon.gameObject.transform.Rotate(0f, 0f, -360 / 10);
            nextAnimateTime = Time.time + 0.1f;
        }
    }

    public void UpdateDisconnected()
    {
        SetStatusIcon("Getrennt");
        ChangeStatusButton(true, "Verbinden");
        //HidePanel(functionsPanel);
        ShowSensorOverlay(true);
        //HideButton(refreshBtn);
        ShowPanel(rssiPanel);
    }

    public void UpdateConnecting()
    {
        ChangeStatusButton(false, "Verbinde...");
        SetStatusIcon("Verbinde");
        HidePanel(rssiPanel);
    }

    public void UpdatedConnected(Dictionary<ID, float> sensorValues, float batteryValue, float sdFillPercentage, bool started, uint writtenBytes, bool sdInitialized, bool genuineSD)
    {
        SetStatusIcon("Verbunden");
        ChangeStatusButton(true, "Trennen");
        ShowPanel(functionsPanel);
        ShowSensorOverlay(false);
        ShowButton(refreshBtn);
        ShowSensorValues(sensorValues);
        UpdateSliders(batteryValue, sdFillPercentage, writtenBytes);
        SetStartButton(started);
        SetSDWarningText(sdInitialized, genuineSD);
    }

    public void SwitchScreenTo(string screenName)
    {
        if (screenList[screenName] != null)
        {
            screenList[screenName].SetActive(true);
        }

        foreach (KeyValuePair<string, GameObject> i in screenList)
        {
            if (i.Key != screenName)
            {
                i.Value.SetActive(false);
            }
        }
    }

    public void ShowPanel(GameObject panel)
    {
        panel.SetActive(true);
    }

    public void HidePanel(GameObject panel)
    {
        panel.SetActive(false);
    }


    public void ShowSensorOverlay(bool v)
    {
        sensorOverlay.enabled = v;
        sensorOverlay.gameObject.SetActive(v);
    }

    public void OpenPopup(GameObject popup)
    {
        eggObj.SetActive(false);
        popupScreen.SetActive(true);
        popup.SetActive(true);
    }

    public void ClosePopup(GameObject popup)
    {
        eggObj.SetActive(true);
        popupScreen.SetActive(false);
        popup.SetActive(false);
    }

    public void HideButton(Button btn)
    {
        btn.gameObject.SetActive(false);
    }

    public void ShowButton(Button btn)
    {
        btn.gameObject.SetActive(true);
    }

    public void SetButtonEnabled(Button btn, bool value)
    {
        if (value)
        {
            btn.GetComponentInChildren<Text>().color = Color.black;
        }
        else
        {
            btn.GetComponentInChildren<Text>().color = Color.grey;
        }
        btn.interactable = value;
    }

    public void SetButtonIconEnabled(Button btn, bool value)
    {
        if (value)
        {
            btn.GetComponent<Image>().color = Color.white;
        }
        else
        {
            btn.GetComponent<Image>().color = Color.gray;
        }
        btn.interactable = value;
    }

    public void SetButtonPressed(Button btn, bool value)
    {
        var colors = btn.GetComponent<Image>().color;
        if (value)
        {
            colors = new Color32(175, 175, 175, 255);
        }
        else
        {
            colors = new Color32(255, 255, 255, 255);
        }
        btn.GetComponent<Image>().color = colors;
        btn.interactable = !value;
    }

    public void SetButtonColor(Button btn, Color col)
    {
        btn.GetComponent<Image>().color = col;
    }

    public void SetStatusText(string value)
    {
        StatusText.text = value;
    }

    public void SetStatusIcon(string value)
    {
        if (value.Equals("Getrennt"))
        {
            disconnectedIcon.gameObject.SetActive(true);
            connectedIcon.gameObject.SetActive(false);
            connectingIcon.gameObject.SetActive(false);
            animatedIcon = false;
        }
        else if (value.Equals("Verbunden"))
        {
            disconnectedIcon.gameObject.SetActive(false);
            connectedIcon.gameObject.SetActive(true);
            connectingIcon.gameObject.SetActive(false);
            animatedIcon = false;
        }
        else if (value.Equals("Verbinde"))
        {
            disconnectedIcon.gameObject.SetActive(false);
            connectedIcon.gameObject.SetActive(false);
            connectingIcon.gameObject.SetActive(true);
            animatedIcon = true;
        }
    }

    public void animateConnectingIcon()
    {
        connectingIcon.gameObject.SetActive(true);
        animatedIcon = true;
    }

    /// <summary>
    /// Updates the RSSI-connectivity-icon on Disconnected-Screen
    /// <br> - Crops the icon linear to the current RSSI-value </br>
    /// <br> - Hides connectButton on RSSI-timeout </br>
    /// </summary>
    public void UpdateRSSI(float rssiValue, DateTime lastRSSITime)
    {
        TimeSpan difference = DateTime.Now.Subtract(lastRSSITime);
        if (!(difference.TotalSeconds > RSSI_TIMEOUT_S) && rssiValue > -(RSSI_MIN))
        {
            SetStatusText("Sensor-Ei: ");
            ShowButton(statusBtn);
            rssiValue = Mathf.Abs(rssiValue);
            rssiIcon.fillAmount = Utility.RemapClamped(rssiValue, RSSI_MAX, RSSI_MIN, 1.0f, 0.17f);
            rssiText.text = "-" + rssiValue.ToString("0") + "dBm";
        }
        else
        {
            SetStatusText("Sensor-Ei: Nicht erreichbar");
            HideButton(statusBtn);
            rssiIcon.fillAmount = 0.0f;
            rssiText.text = "";
        }
    }

    public void UpdateSliders(float bat, float sd, uint writtenBytes)
    {
        spaceSlider.value = sd;
        batSlider.value = bat;
        SDSpaceText.text = (writtenBytes / 1000000000.0f).ToString("00.00") + " GB";
        BatValueText.text = (bat * 100).ToString("0") + "%";
    }

    public void ChangeStatusButton(bool onoff, string text)
    {
        if (onoff)
        {
            statusBtn.GetComponentInChildren<Text>().color = Color.black;
        }
        else
        {
            statusBtn.GetComponentInChildren<Text>().color = Color.grey;
        }
        statusBtn.GetComponentInChildren<Text>().text = text;
        statusBtn.interactable = onoff;
    }

    public void ShowText(Text t)
    {
        t.gameObject.SetActive(true);
    }

    public void HideText(Text t)
    {
        t.gameObject.SetActive(false);
    }

    public void SetStartButton(bool started)
    {
        if (!started)
        {
            startedBtn.GetComponentInChildren<Text>().text = "Start";
            ShowButton(startedBtn);
        }
        else
        {
            startedBtn.GetComponentInChildren<Text>().text = "Beenden";
            ShowButton(startedBtn);
        }
    }

    public void SetSDWarningText(bool sdInitialized, bool genuineSD)
    {
        if (!(sdInitialized && genuineSD))
        {
            ShowText(SDProblemWarningText);

            if (!sdInitialized)
            {
                SDProblemWarningText.text = "Keine SD-Karte gefunden!";
                SDProblemWarningText.color = Color.red;
                SetButtonEnabled(startedBtn, false);
            }
            else if (!genuineSD)
            {
                SDProblemWarningText.text = "Es wurde keine originale SD-Karte eingelegt!";
                SDProblemWarningText.color = Color.black;
                SetButtonEnabled(startedBtn, true);
            }
        }
        else
        {
            HideText(SDProblemWarningText);
            SetButtonEnabled(startedBtn, true);
        }
    }

    public void SetIntervalDropdown(int interval)
    {
        int index = dropInterval.options.FindIndex((i) => { return i.text.Equals(interval.ToString()); });
        dropInterval.value = index;
    }

    /// <summary>
    /// Displays the sensor-values on UI in sensor-panel:
    /// <br> - Checks the Dictionary for values </br>
    /// <br> - Shows the values, if they are available </br>
    /// </summary>
    public void ShowSensorValues(Dictionary<ID, float> values)
    {
        float temp = values.TryGetValue(ID.TEMP, out temp) ? temp : -1;
        TempText.text = temp.ToString("0.00") + "°C";

        float humi = values.TryGetValue(ID.HUMI, out humi) ? humi : -1;
        HumiText.text = humi.ToString("0.00") + "%";

        float onBoardLight = values.TryGetValue(ID.LIGHT, out onBoardLight) ? onBoardLight : -1;
        float light1 = values.TryGetValue(ID.LIGHTANAONE, out light1) ? light1 : -1;
        float light2 = values.TryGetValue(ID.LIGHTANATWO, out light2) ? light2 : -1;

        LightText.text = light2.ToString("0") + "%" + " und " + light1.ToString("0") + "%";

        float value;
        if (values.TryGetValue(ID.ACCX, out value)) { }
        else
        { return; }
        ShowEggOrientation(values[ID.ACCX], values[ID.ACCY], values[ID.ACCZ]);
    }

    /// <summary>
    /// Applies the eggs ACC-values (x,y,z) to the textured Egg-3D-model:
    /// <br> - Uses euler yaws: Roll and Pitch </br>
    /// <br> - Yaw can only be obtained by using the magnetometer </br>
    /// </summary>
    private void ShowEggOrientation(float X, float Y, float Z)
    {
        int sign = 0;
        if (Z > 0)
        {
            sign = +1;
        }
        else
        {
            sign = -1;
        }
        float miu = 0.001f;

        float Roll = Mathf.Atan2(Y, sign * Mathf.Sqrt(Z * Z + miu * X * X)) * 180 / Mathf.PI;
        float Pitch = -(Mathf.Atan2(-X, Mathf.Sqrt(Y * Y + Z * Z)) * 180 / Mathf.PI);

        ElectronicEgg.PrintLog("Roll: " + Roll.ToString());
        ElectronicEgg.PrintLog("Pitch: " + Pitch.ToString());

        eggObj.transform.rotation = eggOrigRotation;
        eggObj.transform.Rotate(Pitch, 0, 0);
        eggObj.transform.Rotate(0, 0, Roll);
    }

    /// <summary>
    /// Updates the hourToggles' color to:
    /// <br> - The current state on egg </br>
    /// <br> - The changes selected by user </br>
    /// <br> - A preset from the Dropdown-menu </br>
    /// </summary>
    public void UpdateToggles(MODEOFHOUR[] hourModes, MODEOFHOUR[] runningModes)
    {
        for (int i = 0; i < hourModes.Length; i++)
        {
            switch (hourModes[i])
            {
                case MODEOFHOUR.OFF:
                    hourToggles[i].transform.GetChild(0).GetComponent<Image>().color = new Color32(255, 255, 255, 255);
                    break;
                case MODEOFHOUR.AUDIO:
                    hourToggles[i].transform.GetChild(0).GetComponent<Image>().color = new Color32(237, 144, 8, 255);
                    break;
                case MODEOFHOUR.BLE:
                    hourToggles[i].transform.GetChild(0).GetComponent<Image>().color = new Color32(2, 89, 208, 255);
                    break;
                default:
                    Console.WriteLine("Default case");
                    break;
            }
        }

        UpdateWarningText(hourModes);
        UpdateSubmitButton(hourModes, runningModes);
        UpdateTogglesDropdown(hourModes);
    }

    /// <summary>
    /// Hides the submit-button if:
    /// <br> - There are no changes to running state on egg no BLE is selected </br>
    /// <br> - No BLE is selected </br>
    /// </summary>
    private void UpdateSubmitButton(MODEOFHOUR[] hourModes, MODEOFHOUR[] runningModes)
    {
        if (!hourModes.Contains(MODEOFHOUR.BLE) || hourModes.SequenceEqual(runningModes))
        {
            SetButtonEnabled(submitBtn, false);
        }
        else
        {
            SetButtonEnabled(submitBtn, true);
        }
    }

    /// <summary>
    /// Shows a warning text if no BLE-hour is selected
    /// </summary>
    private void UpdateWarningText(MODEOFHOUR[] hourModes)
    {
        if (!hourModes.Contains(MODEOFHOUR.BLE))
        {
            ShowText(BLEWarningText);
        }
        else
        {
            HideText(BLEWarningText);
        }
    }

    /// <summary>
    /// Creates the hourToggles using a prefab
    /// </summary>
    public void CreateToggles()
    {
        for (int i = 0; i < 24; i++)
        {
            int z = i;
            CreateToggleTexts(z);
            GameObject g = (GameObject)Instantiate(togglePrefab);
            g.transform.SetParent(togglePanel.GetComponent<RectTransform>(), false);
            g.SetActive(true);

            g.transform.localScale = Vector3.one;

            Toggle t = g.GetComponent<Toggle>();
            t.isOn = false;

            hourToggles[i] = t;
            ElectronicEgg.PrintLog("Created Toggle: " + i);
        }
        togglesCreated = true;
    }

    /// <summary>
    /// Adds the hour-texts to the timetable using a prefab
    /// </summary>
    private void CreateToggleTexts(int z)
    {
        GameObject newGO = (GameObject)Instantiate(txtPrefab);
        newGO.transform.SetParent(togglePanel.GetComponent<RectTransform>(), false);
        newGO.SetActive(true);
        Text myText = newGO.GetComponent<Text>();
        myText.text = z.ToString();
    }

    /// <summary>
    /// Compares the current timetable to two presets (Sensor-only and Audio)
    /// <br> - The preset-references must be synchroniced with Arduinos default tables </br>
    /// <br> - The dropdown menu is updated, if a reference is currently selected </br>
    /// </summary>
    void UpdateTogglesDropdown(MODEOFHOUR[] hourModes)
    {
        byte[] sensorReference = new byte[24];
        byte[] audioReference = new byte[24];
        for (int i = 0; i < 24; i++)
        {
            if (i == defBLEHour[0] || i == defBLEHour[1])
            {
                sensorReference[i] = (byte)MODEOFHOUR.BLE;
                audioReference[i] = (byte)MODEOFHOUR.BLE;
            }
            else
            {
                sensorReference[i] = (byte)MODEOFHOUR.OFF;
                audioReference[i] = (byte)MODEOFHOUR.AUDIO;
            }
        }

        byte[] currentState = new byte[24];
        for (int i = 0; i < 24; i++)
        {
            currentState[i] = (byte)hourModes[i];
        }

        if (currentState.SequenceEqual(sensorReference))
        {
            dropModes.SetValueWithoutNotify(0);
        }
        else if (currentState.SequenceEqual(audioReference))
        {
            dropModes.SetValueWithoutNotify(1);
        }
        else
        {
            dropModes.SetValueWithoutNotify(2);
        }
    }

    /// <summary>
    /// The timespan untill the next BLE hour is displayed in a textfield  
    /// </summary>
    public void UpdateBLECountdown(TimeSpan dif)
    {
        if (dif.Hours > 1)
        {
            NextBLETimeText.text = "Nächste Verbindungsmöglichkeit in " + dif.ToString(@"hh\:mm\:ss");
        }
        else
        {
            NextBLETimeText.text = "Verbindung noch für " + dif.ToString(@"mm\:ss") + " min möglich";
        }
    }
}
