using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class TitleUI : MonoBehaviour {

    public InputField IpInput;
    public InputField PortInput;
    public Toggle OfflineModeToggle;

    public InputField NameInput;
    public Button StartButton;

    public GameLogic GameLogic;
    public NetworkModule NetworkModule;

    // Use this for initialization
    void Start () {
        DontDestroyOnLoad(GameLogic.gameObject);
        DontDestroyOnLoad(NetworkModule.gameObject);

        StartButton.onClick.AddListener(onClickStart);

        IpInput.text = NetworkModule.SERVER_IP;
        PortInput.text = NetworkModule.SERVER_PORT.ToString();
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    void onClickStart()
    {
        bool isOfflineMode = OfflineModeToggle.enabled;

        GameLogic.Instance.entryName = NameInput.text;
        GameLogic.Instance.isTestMode = isOfflineMode;

        if (!isOfflineMode)
        {
            string ip = IpInput.text;
            int port = int.Parse(PortInput.text);
            if (!NetworkModule.instance.Initializer(ip, port))
            {
                return;
            }
        }

        SceneManager.LoadScene(1);
    }
}
