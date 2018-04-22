using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class TouchDirectionInterface : MonoBehaviour {

    TouchButton ButtonUp;
    TouchButton ButtonDown;
    TouchButton ButtonLeft;
    TouchButton ButtonRight;

    private void Awake()
    {
        ButtonUp = transform.Find("Up").gameObject.GetComponent<TouchButton>();
        ButtonDown = transform.Find("Down").gameObject.GetComponent<TouchButton>();
        ButtonLeft = transform.Find("Left").gameObject.GetComponent<TouchButton>();
        ButtonRight = transform.Find("Right").gameObject.GetComponent<TouchButton>();
    }

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public InputDirection GetCurrentDirection()
    {
        if (ButtonLeft.CurrentState)
            return InputDirection.Left;
        else if (ButtonRight.CurrentState)
            return InputDirection.Right;

        else return InputDirection.None;
    }
}
