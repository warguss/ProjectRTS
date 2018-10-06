using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

public class PlayerInventory
{
    public delegate void AddItemDelegate(WeaponId weaponId, int amount);
    public delegate void DeleteItemDelegate(WeaponId weaponId);
    public delegate void SetAmmoDelegate(WeaponId weaponId, int amount);
    public delegate void ClearItemsDelegate();

    public event AddItemDelegate AddItemEvent;
    public event DeleteItemDelegate DeleteItemEvent;
    public event SetAmmoDelegate SetAmmoEvent;
    public event ClearItemsDelegate ClearItemsEvent;


    private ControllableCharacter owner; //필요없는 구조로 개선 예정
    private List<PlayerWeapon> items = new List<PlayerWeapon>();
    public WeaponId currentWeaponId = WeaponId.None;

    public void InvokeAddItemEvent(WeaponId weaponId, int amount)
    {
        AddItemEvent?.Invoke(weaponId, amount);
    }
    public void InvokeDeleteItemEvent(WeaponId weaponId)
    {
        DeleteItemEvent?.Invoke(weaponId);
    }
    public void InvokeSetAmmoEvent(WeaponId weaponId, int amount)
    {
        SetAmmoEvent?.Invoke(weaponId, amount);
    }
    public void InvokeClearItemsEvent()
    {
        ClearItemsEvent?.Invoke();
    }

    public PlayerInventory(ControllableCharacter inOwner)
    {
        owner = inOwner;
    }

    public void AddItem(WeaponId weaponId, int amount)
    {
        int itemIndex = FindWeaponIndex(weaponId);
        if (itemIndex == -1)
        {
            PlayerWeapon weapon = new PlayerWeapon(weaponId, amount, owner);
            items.Add(weapon);
            currentWeaponId = weaponId;
            InvokeAddItemEvent(weaponId, amount);
        }
        else
        {
            items[itemIndex].AddAmmo(amount);
            InvokeSetAmmoEvent(weaponId, items[itemIndex].CurrentAmmo);
        }
    }

    public void AddItem(WeaponId weaponId)
    {
        AddItem(weaponId, WeaponDatabase.Instance.GetDefaultWeaponStat(weaponId).Ammo);
    }

    public void DeleteItem(WeaponId weaponId)
    {
        int itemIndex = FindWeaponIndex(weaponId);
        if (itemIndex != -1)
        {
            items.RemoveAt(itemIndex);
            InvokeDeleteItemEvent(weaponId);
        }
        if(currentWeaponId == weaponId)
        {
            if(items.Count==0)
            {
                currentWeaponId = WeaponId.None;
            }
            else
            {
                currentWeaponId = items[0].WeaponId;
            }
        }
    }

    public void ClearItem()
    {
        items.Clear();
        currentWeaponId = WeaponId.None;
        InvokeClearItemsEvent();
    }

    public int GetCurrentWeaponIndex()
    {
        return FindWeaponIndex(currentWeaponId);
    }

    public void ChangeWeapon(WeaponId weaponId)
    {
        int index = FindWeaponIndex(weaponId);
        if(index != -1)
        {
            currentWeaponId = weaponId;
        }
    }

    public WeaponId ChangeToNextWeapon()
    {
        int nextIndex = (GetCurrentWeaponIndex() + 1) % GameLogic.WEAPON_SLOT_COUNT;
        if (nextIndex >= items.Count)
            nextIndex = 0;

        WeaponId NextWeapon = items[nextIndex].WeaponId;
        ChangeWeapon(NextWeapon);
        return NextWeapon;
    }

    public PlayerWeapon GetCurrentWeapon()
    {
        int index = GetCurrentWeaponIndex();
        if (index == -1)
        {
            return null;
        }
        else
        {
            return items[index];
        }
    }

    public void UpdateWeaponInterval(float deltaTime)
    {
        foreach (var playerWeapon in items) // 연사 딜레이 계산
        {
            playerWeapon.UpdateInterval(deltaTime);
        }
    }

    private int FindWeaponIndex(WeaponId weaponId)
    {
        for(int i=0; i<items.Count; i++)
        {
            if (items[i].WeaponId == weaponId)
                return i;
        }
        return -1;
    }

    
}