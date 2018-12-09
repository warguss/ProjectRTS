using UnityEngine;
using UnityEditor;

public class ShootInfo
{
    public WeaponId HitType;
    public float BulletSpeed;
    public float BulletRange;
    public float shootAngle;//unuse when hit()
    public int Damage;
    public int HitRecovery;// currently unuse
    public int Impact;
}