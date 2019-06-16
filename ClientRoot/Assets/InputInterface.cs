using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum InputDirection
{
    None,
    Up,
    Down,
    Left,
    Right
}

public class InputInterface : MonoBehaviour {

    TouchDirectionInterface directionInterface;
    TouchButtonsInterface buttonInterface;

    private void Awake()
    {
        
    }

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void Initialize()
    {
        directionInterface = transform.Find("TouchInputCanvas/DirectionCanvas").gameObject.GetComponent<TouchDirectionInterface>();
        buttonInterface = transform.Find("TouchInputCanvas/ButtonCanvas").gameObject.GetComponent<TouchButtonsInterface>();
    }

    public InputDirection GetCurrentDirection()
    {
        if (Input.GetKey(KeyCode.LeftArrow))
        {
            return InputDirection.Left;
        }

        else if (Input.GetKey(KeyCode.RightArrow))
        {
            return InputDirection.Right;
        }

        else
        {
            return directionInterface.GetCurrentDirection();
        }
    }

    public bool GetJump()
    {
        if (Input.GetKeyDown(KeyCode.UpArrow))
            return true;

        else
        {
            return buttonInterface.GetJumpDown();
        }
    }

    public bool GetFire()
    {
        if (Input.GetKeyDown(KeyCode.RightShift))
            return true;

        else
        {
            return buttonInterface.GetFireDown();
        }
    }

    public bool GetRoll()
    {
        if (Input.GetKeyDown(KeyCode.DownArrow))
            return true;

        else
        {
            return false;
            //return buttonInterface.GetRollDown();
        }
    }

    public bool GetNextWeapon()
    {
        if (Input.GetKeyDown(KeyCode.Slash))
            return true;

        else
        {
            return buttonInterface.GetWeaponChangeDown();
        }
    }
}
