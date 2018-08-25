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

    public Dictionary<WeaponId, WeaponInfo> Weapons;

    WeaponDatabase()
    {
        Weapons = new Dictionary<WeaponId, WeaponInfo>
        {
            { WeaponId.Pistol, new WeaponInfo(10, 15, 10, 0, 50, 0) },
            { WeaponId.Sniper, new WeaponInfo(15, 20, 15, 0, 60, 0) }
        };
    }
}
