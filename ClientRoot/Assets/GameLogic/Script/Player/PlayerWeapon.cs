using UnityEngine;
using System.Collections;

public class PlayerWeapon
{
    private WeaponId weaponId;
    private WeaponStat stat;
    private ControllableCharacter owner;

    private int currentAmmo = -1;
    private float currentInterval = 0;

    public void UpdateInterval(float deltaTime)
    {
        if (currentInterval > 0)
            currentInterval -= deltaTime;
        if (currentInterval < 0)
            currentInterval = 0;
    }

    public PlayerWeapon(WeaponId id, ControllableCharacter owner)
    {
        SetWeaponInfo(id, owner);
    }

    public bool IsShootable()
    {
        if ((currentAmmo > 0 || currentAmmo == -1)&& currentInterval == 0)
            return true;
        else
            return false;
    }

    public Bullet Shoot(DamageInfo info, Vector2 position)
    {
        GameObject bullet = BulletFactory.Instance.InstantiateBullet(weaponId);
        bullet.transform.position = new Vector3(position.x, position.y);
        Bullet bulletScript = bullet.GetComponent<Bullet>();
        bulletScript.Initialize(owner.OwnerId, stat);

        if (info != null)//For Non local Player
        {
            bulletScript.DamageInfo = info;
        }

        else
        {
            int shootAngle;

            if (owner.IsLeft)
                shootAngle = 180;
            else
                shootAngle = 0;

            bulletScript.SetAngle(shootAngle);
        }

        currentInterval = stat.RapidInterval;

        return bulletScript;
    }

    void SetWeaponInfo(WeaponId id, ControllableCharacter inOwner)
    {
        weaponId = id;
        stat = WeaponDatabase.Instance.GetDefaultWeaponStat(id);
        owner = inOwner;
    }
}
