using UnityEngine;
using System.Collections;

public static class WeaponConstants
{
    public const float DEFAULT_BULLET_SPEED = 10;
    public const float DEFAULT_RANGE = 15;
    public const int DEFAULT_DAMAGE = 20;
    public const int DEFAULT_HIT_RECOVERY = 0;// currently unuse
    public const int DEAFULT_IMPACT = 50;
    public const int DEFAuLT_IMPACT_ANGLE = 0;
    public const float DEFAULT_RAPID_INTERVAL = 0;
}

public enum WeaponId : int
{
    Pistol = 0,
    Sniper = 1,
}

public class WeaponStat
{
    public float BulletSpeed = WeaponConstants.DEFAULT_BULLET_SPEED;
    public float Range = WeaponConstants.DEFAULT_RANGE;
    public int Damage = WeaponConstants.DEFAULT_DAMAGE;
    public int HitRecovery = WeaponConstants.DEFAULT_HIT_RECOVERY;// currently unuse
    public int Impact = WeaponConstants.DEAFULT_IMPACT;
    public int ImpactAngle = WeaponConstants.DEFAuLT_IMPACT_ANGLE;
    public float RapidInterval = WeaponConstants.DEFAULT_RAPID_INTERVAL;

    public WeaponStat()
    {
        
    }

    public WeaponStat(float inBulletSpeed, float inRange, int inDamage, int inHitRecovery, int inImpact, int inImpactAngle, float inRapidInterval)
    {
        BulletSpeed = inBulletSpeed;
        Range = inRange;
        Damage = inDamage;
        HitRecovery = inHitRecovery;
        Impact = inImpact;
        ImpactAngle = inImpactAngle;
        RapidInterval = inRapidInterval;
    }
}
