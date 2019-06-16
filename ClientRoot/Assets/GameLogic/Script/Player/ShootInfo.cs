using UnityEngine;
using UnityEditor;

public class ShootInfo
{
    public WeaponId ShootType;
    public float BulletSpeed;
    public float BulletRange;
    public float ShootAngle;
    public float Damage;
    public float ImpactScale;
}

public class HitInfo
{
    public WeaponId HitType;
    public float Damage;
    public float ImpactX;
    public float ImpactY;
}