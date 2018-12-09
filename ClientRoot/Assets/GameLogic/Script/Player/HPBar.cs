using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class HPBar : MonoBehaviour {

    public SpriteRenderer HPRemaining;

    public void SetPercentage(float percentage)
    {
        var originalSize = HPRemaining.size;
        HPRemaining.size = new Vector2(percentage, originalSize.y);
    }
}
