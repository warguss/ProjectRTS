using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public enum ItemType : int
{
    Recover = 0,
    Weapon = 1,
    
}

public class FieldItem : MonoBehaviour {

    public SpriteRenderer spriteRenderer;

    public string ItemId;

    public ItemType ItemType;
    public WeaponId WeaponId;
    public int Amount;

    bool LocalPlayerGet = false;

	// Use this for initialization
	void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void Initialize(string inItemId, ItemType inItemType, WeaponId inWeaponId, int inAmount)
    {
        ItemId = inItemId;
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
            if (GameLogic.Instance.myId == targetPlayer.OwnerId) // ItemGet패킷 보낸 후 응답을 받아야 먹은 걸로 처리해야 함
            {
                if (!GameLogic.Instance.isOnline)
                {
                    targetPlayer.GetItem(this);
                    Destroy(gameObject);
                }
                else
                {
                    if (!LocalPlayerGet)
                    {
                        LocalPlayerGet = true;
                        NetworkModule.instance.WriteEventGetItem(targetPlayer.OwnerId, ItemId); // 여기서 NetworkModule 불려도 될까?
                    }
                }
            }
        }
    }
}
