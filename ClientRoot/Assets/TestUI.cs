using UnityEngine;
using System.Collections;
using UnityEngine.UI;

public class TestUI : MonoBehaviour {

    public static TestUI Instance;

    public Text Console;
    public ScrollRect scrollView;
    public InputField IpInput;
    public InputField PortInput;
    public Button ConnectButton;

    string consoleMessage;

	// Use this for initialization
	void Start () {
        Instance = this;
        ConnectButton.onClick.AddListener(OnClickConnect);
	}
	
	// Update is called once per frame
	void Update () {
	
	}

    public void PrintText(string message)
    {
        consoleMessage += (message + "\n");
        Console.text = consoleMessage;
        scrollView.verticalScrollbar.value = 0;
    }

    void OnClickConnect()
    {
        string ip = IpInput.text;
        int port = int.Parse(PortInput.text);
        NetworkModule.instance.Connect(ip, port);
    }
}
