﻿using System.Collections;
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
            { WeaponId.Pistol, new WeaponStat(10, 15, 10, 0, 50, 0, 0.25f) },
            { WeaponId.Sniper, new WeaponStat(15, 20, 15, 0, 60, 0, 0.5f) }
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
