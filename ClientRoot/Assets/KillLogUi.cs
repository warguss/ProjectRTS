using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class KillLogUi : MonoBehaviour {

    public static KillLogUi Instance;
    public GameObject killLogTextPrefab; 
    public Button testButton;
    // Use this for initialization
    void Start () {
        Instance = this;
        testButton.onClick.AddListener(OnClickTestButton);
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void GenerateKillLog(string performerName, string victimName)
    {
        GameObject killLogObject = Instantiate(killLogTextPrefab, transform);
        Text killLogText = killLogObject.GetComponent<Text>();
        killLogText.text = string.Format("{0} kill {1}", performerName, victimName);
        if(this.transform.childCount > 3)
        {
            GameObject removedObject = this.transform.GetChild(0).gameObject;
            Destroy(removedObject);
        }
    }

    void OnClickTestButton()
    {
        GenerateKillLog("test0", "test0");
    }
}
