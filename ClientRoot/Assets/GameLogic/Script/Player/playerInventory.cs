using UnityEngine;
using UnityEditor;
using System.Collections.Generic;

public class PlayerInventory
{
    private ControllableCharacter owner; //필요없는 구조로 개선 예정
    private List<PlayerWeapon> items = new List<PlayerWeapon>();
    public WeaponId currentWeaponId = WeaponId.None;

    public PlayerInventory(ControllableCharacter inOwner)
    {
        owner = inOwner;
    }

    public void AddItem(WeaponId weaponId, int amount = 0)
    {
        int itemIndex = FindWeaponIndex(weaponId);
        if (itemIndex == -1)
        {
            PlayerWeapon weapon = new PlayerWeapon(weaponId, owner);
            items.Add(weapon);
            currentWeaponId = weaponId;
        }
        else
        {
            items[itemIndex].AddAmmo(amount);
        }
    }

    public void DeleteItem(WeaponId weaponId)
    {
        int itemIndex = FindWeaponIndex(weaponId);
        if (itemIndex != -1)
        {
            items.RemoveAt(itemIndex);
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