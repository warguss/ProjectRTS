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
        string killLogString = string.Format("{0} kill {1}", performerName, victimName);

        Text killLogText = killLogObject.GetComponent<Text>();
        killLogText.text = killLogString;
        Debug.Log(killLogString);

        if(this.transform.childCount > 3)
        {
            GameObject removedObject = this.transform.GetChild(0).gameObject;
            Destroy(removedObject);
        }
    }

    void OnClickTestButton()
    {
        string performerName = string.Format("test {0}", Random.Range(0, 100));
        string victimName = string.Format("test {0}", Random.Range(0, 100));
        GenerateKillLog(performerName, victimName);
    }
}
