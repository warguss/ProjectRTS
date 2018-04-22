using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TouchButtonsInterface : MonoBehaviour {

    TouchButton ButtonJump;
    TouchButton ButtonFire;

    bool JumpInput = false;
    bool FireInput = false;

    private void Awake()
    {
        ButtonJump = transform.Find("Jump").gameObject.GetComponent<TouchButton>();
        ButtonFire = transform.Find("Fire").gameObject.GetComponent<TouchButton>();
    }

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if (ButtonJump.CurrentState)
            JumpInput = true;

        if (ButtonFire.CurrentState)
            FireInput = true;
    }

    public bool GetJumpDown()
    {
        if(JumpInput)
        {
            JumpInput = false;
            return true;
        }
        return false;
    }

    public bool GetFireDown()
    {
        if (FireInput)
        {
            FireInput = false;
            return true;
        }
        return false;
    }
}
