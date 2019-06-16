using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponUI : MonoBehaviour {

    public static WeaponUI Instance;

    public GameObject WeaponSlotPrefab;

    WeaponSlot[] weaponSlots;

    private void Awake()
    {
        Instance = this;
        weaponSlots = new WeaponSlot[GameLogic.WEAPON_SLOT_COUNT];

        for (int i = 0; i < GameLogic.WEAPON_SLOT_COUNT; i++)
        {
            GameObject slotObject = Instantiate(WeaponSlotPrefab);
            WeaponSlot slotScript = slotObject.GetComponent<WeaponSlot>();

            slotObject.transform.SetParent(transform);
            weaponSlots[i] = slotScript;
            slotScript.SetEmpty();
        }
    }

    // Use this for initialization
    void Start () {
		
	}
	
	// Update is called once per frame
	void Update () {
		
	}

    public void AddWeapon(WeaponId weaponId, int ammo)
    {
        for(int i=0; i<GameLogic.WEAPON_SLOT_COUNT; i++)
        {
            if(weaponSlots[i].WeaponId == -1)
            {
                weaponSlots[i].SetWeapon(weaponId);
                SetAmmo(weaponId, ammo);
                return;
            }
            else if(weaponSlots[i].WeaponId == (int)weaponId)
            {
                Debug.LogWarning("WeaponUI Error : Trying to add duplicate weapon");
                return;
            }
        }
        Debug.LogWarning("WeaponUI Error : Trying to add weapon over the limit");
    }

    public void DeleteWeapon(WeaponId weaponId)
    {
        for (int i = 0; i < GameLogic.WEAPON_SLOT_COUNT; i++)
        {
            if (weaponSlots[i].WeaponId == (int)weaponId)
            {
                weaponSlots[i].SetEmpty();
                return;
            }
        }
        Debug.LogWarning("WeaponUI Error : Trying to delete weapon not exist");
    }

    public void SetAmmo(WeaponId weaponId, int ammo)
    {
        for (int i = 0; i < GameLogic.WEAPON_SLOT_COUNT; i++)
        {
            if (weaponSlots[i].WeaponId == (int)weaponId)
            {
                weaponSlots[i].SetRemainingAmmo(ammo);
                return;
            } 
        }
        Debug.LogWarning("WeaponUI Error : Trying to modify ammo of not added weapon");
    }

    public void SetCurrentWeapon(WeaponId weaponId)
    {
        for (int i = 0; i < GameLogic.WEAPON_SLOT_COUNT; i++)
        {
            if (weaponSlots[i].WeaponId == (int)weaponId)
            {
                weaponSlots[i].ActiveCurrentWeapon();
            }
            else
            {
                weaponSlots[i].DisableCurrentWeapon();
            }
        }
    }

    public void ClearWeaponSlots()
    {
        for (int i = 0; i < GameLogic.WEAPON_SLOT_COUNT; i++)
        {
            weaponSlots[i].SetEmpty();
        }
    }

    public void SetInventory(PlayerInventory inventory)
    {
        ClearWeaponSlots();
        inventory.AddItemEvent += AddWeapon;
        inventory.SetAmmoEvent+= SetAmmo;
        inventory.DeleteItemEvent += DeleteWeapon;
        inventory.ClearItemsEvent += ClearWeaponSlots;
        inventory.ChangeItemEvent += SetCurrentWeapon;
    }
}
