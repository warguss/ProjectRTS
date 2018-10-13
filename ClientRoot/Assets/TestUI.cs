using UnityEngine;
using System.Collections;
using UnityEngine.UI;
using System.Collections.Generic;
using System.Text;

public class TestUI : MonoBehaviour {

    public static TestUI Instance;

    public Text Console;
    public ScrollRect scrollView;
    public InputField IpInput;
    public InputField PortInput;
    public InputField NameInput;
    public Button ConnectButton;
    public Button DisconnectButton;

    const int MAX_STRING = 100;
    StringBuilder concatString;
    List<string> consoleMessage;
    bool newMessage = false;

	// Use this for initialization
	void Start () {
        Instance = this;
        concatString = new StringBuilder();
        consoleMessage = new List<string>();

        ConnectButton.onClick.AddListener(OnClickConnect);
        DisconnectButton.onClick.AddListener(OnClickDisconnect);
        IpInput.text = NetworkModule.SERVER_IP;
        PortInput.text = NetworkModule.SERVER_PORT.ToString();
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

    void OnClickConnect()
    {
        string ip = IpInput.text;
        int port = int.Parse(PortInput.text);
        if ( !NetworkModule.instance.Initializer(ip, port) )
        {
            return;
        }

        if ( !NetworkModule.instance.RequestAuthorization() )
        {
            return;
        }

        if (!NetworkModule.instance.isConnected)
        {
            string name = NameInput.text;
            NetworkModule.instance.Connect(name);
        }
    }

    void OnClickDisconnect()
    {
        if (NetworkModule.instance.isConnected)
        {
            NetworkModule.instance.Disconnect();
            GameLogic.Instance.CleanUpGame();
        }
    }
}
