using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum ItemType : int
{
    Weapon = 0,
    Recover = 1,
}

public class FieldItem : MonoBehaviour {

    public SpriteRenderer spriteRenderer;

    public ItemType ItemType;
    public WeaponId WeaponId;
    public int Amount;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void Initialize(ItemType inItemType, WeaponId inWeaponId, int inAmount)
    {
        ItemType = inItemType;
        WeaponId = inWeaponId;
        Amount = inAmount;

        RefreshImage();
    }

    void RefreshImage()
    {
        switch(ItemType)
        {
            case ItemType.Weapon:
                {
                    switch (WeaponId)
                    {
                        case WeaponId.Pistol:
                            {
                                spriteRenderer.color = Color.yellow;
                                break;
                            }
                        case WeaponId.Sniper:
                            {
                                spriteRenderer.color = Color.cyan;
                                break;
                            }
                    }
                    break;
                }
            case ItemType.Recover:
                {
                    spriteRenderer.color = Color.green;
                    break;
                }
        }
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.tag == "Player")
        {
            MainCharacter targetPlayer = other.gameObject.GetComponent<MainCharacter>();
            targetPlayer.GetItem(this);
            Destroy(gameObject);
        }
    }
}
