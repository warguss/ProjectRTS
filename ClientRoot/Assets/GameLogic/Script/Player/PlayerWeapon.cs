using UnityEngine;
using System.Collections;

public class PlayerWeapon
{
    private WeaponStat stat;
    private ControllableCharacter owner;
    
    public int CurrentAmmo { get; private set; }
    public WeaponId WeaponId { get; private set; }

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
        if ((CurrentAmmo > 0 || CurrentAmmo == -1)&& currentInterval == 0)
            return true;
        else
            return false;
    }

    public Bullet Shoot(ShootInfo info, Vector2 position)
    {
        GameObject bullet = BulletFactory.Instance.InstantiateBullet(WeaponId);
        bullet.transform.position = new Vector3(position.x, position.y);
        Bullet bulletScript = bullet.GetComponent<Bullet>();
        bulletScript.Initialize(owner.OwnerId, stat);
        CurrentAmmo -= 1;

        if (info != null)//For Non local Player
        {
            bulletScript.BulletStat = info;
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
        WeaponId = id;
        stat = WeaponDatabase.Instance.GetDefaultWeaponStat(id);
        CurrentAmmo = stat.Ammo;
        owner = inOwner;
    }
}
