using UnityEngine;
using System.Collections;

public class CameraMove : MonoBehaviour {

    public GameObject MainCamera;

    GameObject Target;

	// Use this for initialization
	void Start () {
	
	}
	
	// Update is called once per frame
	void Update () {
        if (Target)
        {
            var myPosition = Target.transform.position;
            MainCamera.transform.position = new Vector3(myPosition.x, myPosition.y, -10);
        }
	}

    public void SetTarget(GameObject inTarget)
    {
        Target = inTarget;
    }
}
