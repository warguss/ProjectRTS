using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.EventSystems;

public class TouchButton : MonoBehaviour, IPointerDownHandler, IPointerUpHandler
{

    public bool CurrentState { get; set; }

    private void Awake()
    {
        CurrentState = false;
    }

    public void OnPointerDown(PointerEventData ped)
    {
        CurrentState = true;
    }
    public void OnPointerUp(PointerEventData ped)
    {
        CurrentState = false;
    }
}
