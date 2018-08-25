using UnityEngine;
using System.Collections;

public enum WeaponId : int
{
    Pistol = 0,
    Sniper = 1,
}

public class WeaponInfo
{
    public float BulletSpeed;
    public float Range;
    public int Damage;
    public int HitRecovery;// currently unuse
    public int Impact;
    public int ImpactAngle;

    public WeaponInfo(float inBulletSpeed, float inRange, int inDamage, int inHitRecovery, int inImpact, int inImpactAngle)
    {
        BulletSpeed = inBulletSpeed;
        Range = inRange;
        Damage = inDamage;
        HitRecovery = inHitRecovery;
        Impact = inImpact;
        ImpactAngle = inImpactAngle;
    }
}
