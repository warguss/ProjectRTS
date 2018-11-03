using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;
using UnityEngine.UI;

public class TouchDirectionInterface : MonoBehaviour, IDragHandler, IPointerDownHandler, IPointerUpHandler
{
    Canvas Canvas;
    CanvasScaler CanvasScaler;
    RectTransform Rect;
    TouchButton ButtonUp;
    TouchButton ButtonDown;
    TouchButton ButtonLeft;
    TouchButton ButtonRight;
    GameObject JoystickHandle;

    float OriginalRadius;
    float CanvasScaleFactor;
    float ScaledRadius;
    Vector2 CenterPosition;

    bool isPressed = false;
    Vector2 CurrentPointPosition = new Vector2(0, 0);

    const float INPUT_THRESHOLD = 50;

    private void Awake()
    {
        Canvas = GetComponent<Canvas>();
        CanvasScaler = transform.parent.GetComponent<CanvasScaler>();
        Rect = GetComponent<RectTransform>();
        ButtonUp = transform.Find("Up").gameObject.GetComponent<TouchButton>();
        ButtonDown = transform.Find("Down").gameObject.GetComponent<TouchButton>();
        ButtonLeft = transform.Find("Left").gameObject.GetComponent<TouchButton>();
        ButtonRight = transform.Find("Right").gameObject.GetComponent<TouchButton>();
        JoystickHandle = transform.Find("JoystickHandle").gameObject;

        OriginalRadius = Rect.rect.width / 2;
        CanvasScaleFactor = Canvas.scaleFactor;
        ScaledRadius = OriginalRadius * CanvasScaleFactor;
        CenterPosition = transform.position;
    }

    // Use this for initialization
    void Start () {
        
    }
	
	// Update is called once per frame
	void Update () {

	}

    public InputDirection GetCurrentDirection()
    {
        return GetCurrentInput();
    }

    void IDragHandler.OnDrag(PointerEventData eventData)
    {
        CurrentPointPosition = PositionNomalize(eventData);
    }

    void IPointerDownHandler.OnPointerDown(PointerEventData eventData)
    {
        CurrentPointPosition = PositionNomalize(eventData);
        isPressed = true;
    }

    void IPointerUpHandler.OnPointerUp(PointerEventData eventData)
    {
        CurrentPointPosition = PositionNomalize(eventData);
        isPressed = false;
        JoystickHandle.GetComponent<Transform>().localPosition = Vector2.zero;
        Debug.Log(JoystickHandle.GetComponent<Transform>().localPosition);
    }

    Vector2 PositionNomalize(PointerEventData eventData)
    {
        Vector2 NomalizedPosition = new Vector2(
            Mathf.Clamp(((eventData.position.x - CenterPosition.x) * 100 / ScaledRadius), -100, 100),
            Mathf.Clamp(((eventData.position.y - CenterPosition.y) * 100 / ScaledRadius), -100, 100)
            );
        Debug.Log(JoystickHandle.GetComponent<Transform>().localPosition);
        JoystickHandle.GetComponent<Transform>().localPosition = NomalizedPosition /2;
        Debug.Log(JoystickHandle.GetComponent<Transform>().localPosition);
        Debug.Log(NomalizedPosition);
        return NomalizedPosition;
    }

    InputDirection GetCurrentInput()
    {
        if (isPressed)
        {
            if (CurrentPointPosition.x > INPUT_THRESHOLD)
            {
                return InputDirection.Right;
            }
            else if (CurrentPointPosition.x < INPUT_THRESHOLD * -1)
            {
                return InputDirection.Left;
            }
            return InputDirection.None;
        }
        else
        {
            return InputDirection.None;
        }
    }

}
