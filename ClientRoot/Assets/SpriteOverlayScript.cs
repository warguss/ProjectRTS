using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class SpriteOverlayScript : MonoBehaviour {

    SpriteRenderer spriteRenderer;

    bool Invincible = false;
    bool InvincibleAnimationPlaying = false;

    const float INVINCIBLE_ANIMATION_INTERVAL = 0.03f;

    // Use this for initialization
    private void Awake()
    {
        spriteRenderer = GetComponent<SpriteRenderer>();
    }

    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {

	}

    public void SetInvincible(bool enable)
    {
        Invincible = enable;
        if(Invincible)
        {
            if (!InvincibleAnimationPlaying)
                StartCoroutine(InvincibleAnimation());
        }
    }

    IEnumerator InvincibleAnimation()
    {
        Color originalColor = spriteRenderer.color;
        InvincibleAnimationPlaying = true;
        while (Invincible)
        {
            spriteRenderer.color = new Color(Color.white.r, Color.white.g, Color.white.b, 0f);
            yield return new WaitForSeconds(INVINCIBLE_ANIMATION_INTERVAL);
            spriteRenderer.color = new Color(Color.white.r, Color.white.g, Color.white.b, 0.5f);
            yield return new WaitForSeconds(INVINCIBLE_ANIMATION_INTERVAL);
        }
        spriteRenderer.color = originalColor;
        InvincibleAnimationPlaying = false;
    }
}
