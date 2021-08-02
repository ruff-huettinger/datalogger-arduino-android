using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Linq;


public class EggUI : MonoBehaviour
{
    public Button statusBtn;
    public Button autoRefreshBtn;
    public Button refreshBtn;
    public Button startedBtn;
    public Button submitBtn;

    /// <summary>
    /// The 3d representation of the acc-values
    /// </summary>
    public GameObject eggObj;
    public Quaternion eggOrigRotation;

    public Text StatusText;
    public Text SensorText;
    public Text RssiText;
    public Text TempText;
    public Text HumiText;
    public Text LightText;
    public Text SensorStateText;
    public Text BLEWarningText;



    // Default texts for the textfields
    private string rssiDefText = "RSSI: ";
    private string humiDefText;
    private string tempDefText;
    private string lightDefText;

    public Slider batSlider;
    public Slider spaceSlider;

    public GameObject mainScreen;
    public GameObject settingsScreen;
    public GameObject popupScreen;


    public GameObject togglePanel;
    public GameObject statusPanel;
    public GameObject sensorPanel;
    public GameObject functionsPanel;

    public Dictionary<string, GameObject> panelsList = new Dictionary<string, GameObject>();

    /// <summary>
    /// Dropdown for selecting the time-interval of sensor-measurings
    /// <para> Will be synchronized by the connected egg </para>
    /// </summary>
    public Dropdown dropInterval;

    /// <summary>
    /// Dropdown for selecting presets of timetables
    /// <para> Will be synchronized by the connected egg </para>
    /// <para> Will be overwritten by user if the toggles are changed </para>
    /// </summary>
    public Dropdown dropModes;

    public Toggle[] modeToggles { get; set; } = new Toggle[24];
    public GameObject togglePrefab;
    public GameObject txtPrefab;
    public bool togglesCreated { get; set; } = false;

    /// <summary>
    /// A toggle-group for selecting audio or ble
    /// <para> The selected value will be applied to the toggles </para>
    /// </summary>
    public Toggle[] selectionToggles;

    /// <summary>
    /// The default BLE-hours for the table-presets
    /// </summary>
    byte[] defBLEHour = { 10, 15 };

    public MODEOFHOUR[] customMode { get; set; } = new MODEOFHOUR[24];

    public GameObject formatWarningPanel;
    public GameObject cancelWarningPanel;


    void Awake()
    {
        panelsList.Add("main", mainScreen);
        panelsList.Add("settings", settingsScreen);
        panelsList.Add("popup", popupScreen);
    }

    // Start is called before the first frame update
    void Start()
    {
        GetDefaultTexts();
        eggOrigRotation = eggObj.transform.rotation;
    }

    // Update is called once per frame
    void Update()
    {

    }

    public void SwitchScreenTo(string screenName)
    {
        if (panelsList[screenName] != null)
        {
            panelsList[screenName].SetActive(true);
        }

        foreach (KeyValuePair<string, GameObject> i in panelsList)
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

    public void SetPanelColor(GameObject panel, Color col)
    {
        panel.GetComponent<Image>().color = col;
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
        btn.interactable = value;
    }

    public void SetButtonColor(Button btn, Color col)
    {
        btn.GetComponent<Image>().color = col;
    }
    public void SetStatusText(string value)
    {
        StatusText.text = value;
    }
    public void SetStateText(string value)
    {
        SensorStateText.text = value;
    }

    public void UpdateRSSI(float rssiValue)
    {
        RssiText.text = rssiDefText + rssiValue + " dBm";
    }

    public void UpdateSliders(float bat, float sd)
    {
        spaceSlider.value = sd;
        batSlider.value = bat;
    }

    public void ChangeStatusButton(bool onoff, string text)
    {
        statusBtn.GetComponentInChildren<Text>().text = text;
        statusBtn.interactable = onoff;
    }

    public void HideRSSIValue()
    {

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
            ShowButton(startedBtn);
            startedBtn.GetComponentInChildren<Text>().text = "Start";
        }
        else
        {
            startedBtn.GetComponentInChildren<Text>().text = "Abbrechen";
            HideButton(startedBtn);
        }
    }

    public void SetIntervalDropdown(int interval)
    {
        int index = dropInterval.options.FindIndex((i) => { return i.text.Equals(interval.ToString()); });
        dropInterval.value = index;
    }

    public void ShowSensorValues(Dictionary<ID, float> values)
    {
        float temp = values.TryGetValue(ID.TEMP, out temp) ? temp : -1;
        TempText.text = tempDefText + " " + temp.ToString("0.00") + "°C";

        float humi = values.TryGetValue(ID.HUMI, out humi) ? humi : -1;
        HumiText.text = humiDefText + " " + humi.ToString("0.00") + "%";

        float onBoardLight = values.TryGetValue(ID.LIGHT, out onBoardLight) ? onBoardLight : -1;
        float light1 = values.TryGetValue(ID.LIGHTANAONE, out light1) ? light1 : -1;
        float light2 = values.TryGetValue(ID.LIGHTANATWO, out light2) ? light2 : -1;

        //LightText.text = lightDefText + " " + onBoardLight.ToString() + " | " + light1.ToString();
        LightText.text = "Licht: Oben: " + light2.ToString("0") + "%" + " " + "Unten: " + light1.ToString("0") + "%";

        float value;
        if (values.TryGetValue(ID.ACCX, out value)) { }
        else
        { return; }
        ShowEggOrientation(values[ID.ACCX], values[ID.ACCY], values[ID.ACCZ]);
    }

    private void ShowEggOrientation(float X, float Y, float Z)
    {
        ElectronicEgg.PrintLog("Setting Egg Pos");
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

        //float Roll = Mathf.Atan2(Y, Z) * 180 / Mathf.PI;
        float Roll = Mathf.Atan2(Y, sign * Mathf.Sqrt(Z * Z + miu * X * X)) * 180 / Mathf.PI;
        float Pitch = -(Mathf.Atan2(-X, Mathf.Sqrt(Y * Y + Z * Z)) * 180 / Mathf.PI);

        ElectronicEgg.PrintLog("Roll: " + Roll.ToString());
        ElectronicEgg.PrintLog("Pitch: " + Pitch.ToString());

        eggObj.transform.rotation = eggOrigRotation;
        eggObj.transform.Rotate(Pitch, 0, 0);
        eggObj.transform.Rotate(0, 0, Roll);
    }

    private void GetDefaultTexts()
    {
        rssiDefText = RssiText.text;
        humiDefText = HumiText.text;
        tempDefText = TempText.text;
        lightDefText = LightText.text;
    }

    public void UpdateToggles(MODEOFHOUR[] hourModes)
    {
        for (int i = 0; i < hourModes.Length; i++)
        {
            switch (hourModes[i])
            {
                case MODEOFHOUR.OFF:
                    modeToggles[i].transform.GetChild(0).GetComponent<Image>().color = Color.gray;
                    break;
                case MODEOFHOUR.AUDIO:
                    modeToggles[i].transform.GetChild(0).GetComponent<Image>().color = Color.red;
                    break;
                case MODEOFHOUR.BLE:
                    modeToggles[i].transform.GetChild(0).GetComponent<Image>().color = Color.blue;
                    break;
                default:
                    Console.WriteLine("Default case");
                    break;
            }
        }

        if (!hourModes.Contains(MODEOFHOUR.BLE))
        {
            ShowText(BLEWarningText);
            SetButtonEnabled(submitBtn, false);
        }
        else
        {
            HideText(BLEWarningText);
            SetButtonEnabled(submitBtn, true);
        }

        UpdateTogglesDropdown(hourModes);
    }

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

            modeToggles[i] = t;
            ElectronicEgg.PrintLog("Created Toggle: " + i);
        }
        togglesCreated = true;
    }

    private void CreateToggleTexts(int z)
    {
        GameObject newGO = (GameObject)Instantiate(txtPrefab);
        newGO.transform.SetParent(togglePanel.GetComponent<RectTransform>(), false);
        newGO.SetActive(true);

        //Vector3 defPos = newGO.transform.localPosition;
        // newGO.transform.localPosition = new Vector3(defPos.x + 500, defPos.y, defPos.z);

        Text myText = newGO.GetComponent<Text>();
        myText.text = z.ToString();
    }

    public void FillToggles(int table, MODEOFHOUR[] hourModes)
    {
        switch (table)
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
        UpdateToggles(hourModes);
    }

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

    public void UpdateCustomMode(MODEOFHOUR[] modes)
    {
        modes.CopyTo(customMode, 0);
    }
}
