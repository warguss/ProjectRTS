using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class TouchButtonsInterface : MonoBehaviour {

    TouchButton ButtonJump;
    TouchButton ButtonFire;
    TouchButton ButtonWeaponChange;

    bool JumpInput = false;
    bool FireInput = false;
    bool WeaponChangeInput = false;

    private void Awake()
    {
        ButtonJump = transform.Find("Jump").gameObject.GetComponent<TouchButton>();
        ButtonFire = transform.Find("Fire").gameObject.GetComponent<TouchButton>();
        ButtonWeaponChange = transform.Find("WeaponChange").gameObject.GetComponent<TouchButton>();
    }

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
        if (ButtonJump.CurrentState)
        {
            JumpInput = true;
            ButtonJump.CurrentState = false;
        }

        if (ButtonFire.CurrentState)
        {
            FireInput = true;
            ButtonFire.CurrentState = false;
        }

        if (ButtonWeaponChange.CurrentState)
        {
            WeaponChangeInput = true;
            ButtonWeaponChange.CurrentState = false;
        }
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

    public bool GetWeaponChangeDown()
    {
        if (WeaponChangeInput)
        {
            WeaponChangeInput = false;
            return true;
        }
        return false;
    }
}
