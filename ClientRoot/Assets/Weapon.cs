using UnityEngine;
using System.Collections;

public enum WeaponId : int
{
    Pistol = 0,
    Sniper = 1,
}

public struct Weapon
{
    int weaponId;
    DamageInfo defaultDamageInfo;
}
