using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class TestUI : MonoBehaviour {

    public static TestUI Instance;

    public Text Console;
    public ScrollRect scrollView;
    public InputField IpInput;
    public InputField PortInput;
    public InputField NameInput;
    public Button ConnectButton;
    public Button DisconnectButton;

    string consoleMessage;
    bool newMessage = false;

	// Use this for initialization
	void Start () {
        Instance = this;
        ConnectButton.onClick.AddListener(OnClickConnect);
        DisconnectButton.onClick.AddListener(OnClickDisconnect);
    }
	
	// Update is called once per frame
	void Update () {
        if (newMessage)
        {
            Console.text = consoleMessage;
            scrollView.verticalScrollbar.value = 0;
            newMessage = false;
        }
	}

    public void PrintText(string message)
    {
        consoleMessage += (message + "\n");
        newMessage = true;
        Debug.Log(message);
    }

    void OnClickConnect()
    {
        string ip = IpInput.text;
        int port = int.Parse(PortInput.text);
        string name = NameInput.text;
        NetworkModule.instance.Connect(ip, port, name);
    }

    void OnClickDisconnect()
    {
        NetworkModule.instance.Disconnect();
    }
}
