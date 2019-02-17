using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using System.Collections.Generic;
using System.Text;

public class TestUI : MonoBehaviour {

    public static TestUI Instance;

    public Text Console;
    public ScrollRect scrollView;
    public Button DisconnectButton;
    public Button CreateItemButton;

    const int MAX_STRING = 100;
    StringBuilder concatString;
    List<string> consoleMessage;
    bool newMessage = false;

	// Use this for initialization
	void Start () {
        Instance = this;
        concatString = new StringBuilder();
        consoleMessage = new List<string>();

        DisconnectButton.onClick.AddListener(OnClickDisconnect);
        //CreateItemButton.onClick.AddListener(OnClickItemCreate);
    }
	
	// Update is called once per frame
	void Update () {
        if (newMessage)
        {
            concatString.Clear();
            for (int i=0; i<consoleMessage.Count; i++)
            {
                concatString.AppendLine(consoleMessage[i]);
            }
            Console.text = concatString.ToString();
            scrollView.verticalScrollbar.value = 0;
            newMessage = false;
        }
	}

    public void PrintText(string message)
    {
        consoleMessage.Add(message);
        if(consoleMessage.Count > MAX_STRING)
        {
            consoleMessage.RemoveAt(0);
        }
        newMessage = true;
        Debug.Log(message);
    }

    void OnClickDisconnect()
    {
        if (NetworkModule.instance.isConnected)
        {
            NetworkModule.instance.Disconnect();
            GameLogic.Instance.CleanUpGame();
        }
    }

    void OnClickItemCreate()
    {
        GameLogic.Instance.CreateItem(new Vector2(3, 3), "test", ItemType.Recover, WeaponId.Pistol);
    }
}
