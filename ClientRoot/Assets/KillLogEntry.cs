using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class KillLogEntry : MonoBehaviour {

	// Use this for initialization
	void Start () {
        Destroy(this.gameObject, 5f);
	}
	
	// Update is called once per frame
	void Update () {
		
	}
}
