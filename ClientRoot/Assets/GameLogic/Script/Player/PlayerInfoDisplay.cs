using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class PlayerInfoDisplay : MonoBehaviour {

    public TextMesh PlayerNameMesh;
    public HPBar HPBar;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {

	}

    public void SetName(string name)
    {
        PlayerNameMesh.text = name;
    }

    public void SetHP(float percentage)
    {
        HPBar.SetPercentage(percentage);
    }
}
