using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

public class PlayerInventory
{
    public delegate void AddItemDelegate(WeaponId weaponId, int amount);
    public delegate void DeleteItemDelegate(WeaponId weaponId);
    public delegate void SetAmmoDelegate(WeaponId weaponId, int amount);
    public delegate void ClearItemsDelegate();
    public delegate void ChangeItemDelegate(WeaponId weaponId);

    public event AddItemDelegate AddItemEvent;
    public event DeleteItemDelegate DeleteItemEvent;
    public event SetAmmoDelegate SetAmmoEvent;
    public event ClearItemsDelegate ClearItemsEvent;
    public event ChangeItemDelegate ChangeItemEvent;


    private ControllableCharacter owner; //필요없는 구조로 개선 예정
    private List<PlayerWeapon> items = new List<PlayerWeapon>();
    private WeaponId m_CurrentWeaponId = WeaponId.None;

    public WeaponId CurrentWeaponId {
        get
        {
            return m_CurrentWeaponId;
        }
        private set
        {
            m_CurrentWeaponId = value;
            ChangeItemEvent?.Invoke(value);
        }
    }

    public void InvokeSetAmmoEvent(WeaponId weaponId, int amount)
    {
        SetAmmoEvent?.Invoke(weaponId, amount);
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
            AddItemEvent?.Invoke(weaponId, amount);
            if (items.Count == 1)
                CurrentWeaponId = weaponId;
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
            DeleteItemEvent?.Invoke(weaponId);
        }
        if(CurrentWeaponId == weaponId)
        {
            if(items.Count==0)
            {
                CurrentWeaponId = WeaponId.None;
            }
            else
            {
                CurrentWeaponId = items[0].WeaponId;
            }
        }
    }

    public void ClearItem()
    {
        items.Clear();
        CurrentWeaponId = WeaponId.None;
        ClearItemsEvent?.Invoke();
    }

    public int GetCurrentWeaponIndex()
    {
        return FindWeaponIndex(CurrentWeaponId);
    }

    public void ChangeWeapon(WeaponId weaponId)
    {
        int index = FindWeaponIndex(weaponId);
        if(index != -1)
        {
            CurrentWeaponId = weaponId;
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