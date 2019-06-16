using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class WeaponDatabase {

    public static WeaponDatabase Instance
    {
        get
        {
            if (m_Instance == null)
                m_Instance =  new WeaponDatabase();

            return m_Instance;
        }
    }
    private static WeaponDatabase m_Instance;

    private Dictionary<WeaponId, WeaponStat> weapons;

    WeaponDatabase()
    {
        weapons = new Dictionary<WeaponId, WeaponStat>
        {
            { WeaponId.Pistol, new WeaponStat(WeaponId.Pistol, 10, 15, 10, 0, 150, 0, 0.2f, 50) },
            { WeaponId.Sniper, new WeaponStat(WeaponId.Sniper, 20, 25, 20, 0, 250, 0, 0.5f, 20) },
            { WeaponId.Bazooka, new WeaponStat(WeaponId.Bazooka, 15, 25, 15, 0, 500, 0, 0.4f, 20) },
        };
    }

    public WeaponStat GetDefaultWeaponStat(WeaponId id)
    {
        if (weapons.ContainsKey(id))
            return weapons[id];
        else
            return new WeaponStat();
    }
}
