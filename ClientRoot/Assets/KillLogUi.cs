using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class KillLogUi : MonoBehaviour {

    public static KillLogUi Instance;
    public GameObject killLogTextPrefab;
    public Transform KillLogPanel;

    // Use this for initialization
    void Start () {
        Instance = this;
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void GenerateKillLog(string performerName, string victimName)
    {
        GameObject killLogObject = Instantiate(killLogTextPrefab, KillLogPanel);
        string killLogString = string.Format("{0} kill {1}", performerName, victimName);

        Text killLogText = killLogObject.GetComponent<Text>();
        killLogText.text = killLogString;
        Debug.Log(killLogString);

        if(KillLogPanel.childCount > 5)
        {
            GameObject removedObject = this.KillLogPanel.GetChild(0).gameObject;
            Destroy(removedObject);
        }
    }
}
