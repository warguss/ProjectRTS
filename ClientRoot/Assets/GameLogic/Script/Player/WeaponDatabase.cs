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
            { WeaponId.Pistol, new WeaponStat(WeaponId.Pistol, 10, 15, 10, 0, 50, 0, 0.2f) },
            { WeaponId.Sniper, new WeaponStat(WeaponId.Sniper, 20, 25, 20, 0, 60, 0, 0.5f) }
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
