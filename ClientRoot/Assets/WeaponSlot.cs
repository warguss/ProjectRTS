using System.Collections;
using System.Collections.Generic;
using UnityEngine;
using UnityEngine.UI;

public class WeaponSlot : MonoBehaviour {

    Image slotBackground;
    Image weaponIcon;
    Text remainingAmmo;
    
    public int WeaponId { get; private set; }
    public bool IsActive { get; private set; }

    private void Awake()
    {
        slotBackground = transform.Find("WeaponSlotBackground").GetComponent<Image>();
        weaponIcon = transform.Find("WeaponSlotBackground/WeaponIcon").GetComponent<Image>();
        remainingAmmo = transform.Find("RemainingAmmo").GetComponent<Text>();
    }

    // Use this for initialization
    void Start () {

    }
	
	// Update is called once per frame
	void Update () {
		
	}

    public void SetEmpty()
    {
        slotBackground.color = Color.grey;
        weaponIcon.gameObject.SetActive(false);
        remainingAmmo.text = "Empty";
        WeaponId = -1;
    }

    public void SetWeapon(WeaponId inWeaponId)
    {
        WeaponId = (int)inWeaponId;
        slotBackground.color = Color.white;
        weaponIcon.gameObject.SetActive(true);
        switch(inWeaponId)
        {
            case global::WeaponId.Pistol:
                weaponIcon.color = Color.yellow;
                break;
            case global::WeaponId.Sniper:
                weaponIcon.color = Color.magenta;
                break;
            default:
                WeaponId = -1;
                break;
        }

    }

    public void SetRemainingAmmo(int ammo)
    {
        if (ammo == -1)
        {
            remainingAmmo.text = "∞";
        }
        else
            remainingAmmo.text = ammo.ToString();
    }

    public void ActiveCurrentWeapon()
    {
        slotBackground.color = Color.red;
        IsActive = true;
    }

    public void DisableCurrentWeapon()
    {
        slotBackground.color = Color.white;
        IsActive = false;
    }
}
