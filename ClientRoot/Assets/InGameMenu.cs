using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;
using UnityEngine.UI;

public class InGameMenu : MonoBehaviour {

    public Button BackToTitleButton;

	// Use this for initialization
	void Start () {
        BackToTitleButton.onClick.AddListener(BackToTitle);
    }
	
	// Update is called once per frame
	void Update () {
		
	}

    void BackToTitle()
    {
        NetworkModule.instance.Disconnect();
        GameLogic.Instance.CleanUpGame();

        SceneManager.LoadScene(0);
    }
}
