using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.SceneManagement;

public class GameSceneManager : MonoBehaviour {

    InputInterface inputInterface;
    CameraMove cameraScript;

    void OnEnable()
    {
        SceneManager.sceneLoaded += OnSceneLoaded;
    }

    // called second
    void OnSceneLoaded(Scene scene, LoadSceneMode mode)
    {
        if (scene.buildIndex == 1)
        {
            inputInterface = GameObject.Find("InputInterface").gameObject.GetComponent<InputInterface>();
            cameraScript = GameObject.Find("CameraMove").gameObject.GetComponent<CameraMove>();

            inputInterface.Initialize();

            GameLogic.Instance.inputInterface = inputInterface;
            GameLogic.Instance.CameraScript = cameraScript;

            GameLogic.Instance.StartGame();
        }
    }

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    void OnDisable()
    {
        SceneManager.sceneLoaded -= OnSceneLoaded;
    }
}
