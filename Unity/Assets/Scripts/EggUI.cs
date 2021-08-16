using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;
using System.Linq;


public class EggUI : MonoBehaviour
{
    private const double RSSI_TIMEOUT_S = 5.0f;

    public Button statusBtn;
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
    public Text TempText;
    public Text HumiText;
    public Text LightText;
    public Text BLEWarningText;



    // Default texts for the textfields
    private string rssiDefText = "RSSI: ";
    private string humiDefText = "";
    private string tempDefText = "";
    private string lightDefText = "";

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
    public GameObject rssiPanel;

    public Image rssiIcon;


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

    public GameObject formatWarningPanel;
    public GameObject cancelWarningPanel;


    void Awake()
    {
        screenList.Add("main", mainScreen);
        screenList.Add("settings", settingsScreen);
        screenList.Add("popup", popupScreen);
    }

    // Start is called before the first frame update
    void Start()
    {
        //GetDefaultTexts();
        eggOrigRotation = eggObj.transform.rotation;
    }

    // Update is called once per frame
    void Update()
    {

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

    public void UpdateRSSI(float rssiValue, DateTime lastRSSITime)
    {
        TimeSpan difference = DateTime.Now.Subtract(lastRSSITime);
        //ElectronicEgg.PrintLog(difference.TotalSeconds.ToString());
        if (!(difference.TotalSeconds > RSSI_TIMEOUT_S))
        {
            ShowButton(statusBtn);
            rssiValue = Mathf.Abs(rssiValue);
            rssiIcon.fillAmount = Utility.RemapClamped(rssiValue, 50, 100, 1.0f, 0.17f);
        }
        else
        {
            HideButton(statusBtn);
            rssiIcon.fillAmount = 0.17f;
        }
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
            startedBtn.GetComponentInChildren<Text>().text = "Start";
            ShowButton(startedBtn);
        }
        else
        {
            startedBtn.GetComponentInChildren<Text>().text = "Abbrechen";
            ShowButton(startedBtn);
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
        humiDefText = HumiText.text;
        tempDefText = TempText.text;
        lightDefText = LightText.text;
    }

    public void UpdateToggles(MODEOFHOUR[] hourModes, MODEOFHOUR[] runningModes)
    {
        for (int i = 0; i < hourModes.Length; i++)
        {
            switch (hourModes[i])
            {
                case MODEOFHOUR.OFF:
                    modeToggles[i].transform.GetChild(0).GetComponent<Image>().color = new Color32(255, 255, 255, 255);
                    break;
                case MODEOFHOUR.AUDIO:
                    modeToggles[i].transform.GetChild(0).GetComponent<Image>().color = new Color32(237, 144, 8, 255);
                    break;
                case MODEOFHOUR.BLE:
                    modeToggles[i].transform.GetChild(0).GetComponent<Image>().color = new Color32(2, 89, 208, 255);
                    break;
                default:
                    Console.WriteLine("Default case");
                    break;
            }
        }

        if (!hourModes.Contains(MODEOFHOUR.BLE) || hourModes.SequenceEqual(runningModes))
        {
            if (!hourModes.Contains(MODEOFHOUR.BLE))
            {
                ShowText(BLEWarningText);
            }
            SetButtonEnabled(submitBtn, false);
            ElectronicEgg.PrintLog("im false");
        }
        else
        {
            HideText(BLEWarningText);
            SetButtonEnabled(submitBtn, true);
            ElectronicEgg.PrintLog("im true");
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
}
