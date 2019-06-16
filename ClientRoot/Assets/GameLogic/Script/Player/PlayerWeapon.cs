using UnityEngine;
using System.Collections;

public class PlayerWeapon
{
    private const int BAZOOKA_DEFAULT_ANGLE = 45; 

    public WeaponStat stat;
    private ControllableCharacter owner; //필요없는 구조로 개선 예정
    
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

    public PlayerWeapon(WeaponId id, int ammo, ControllableCharacter owner)
    {
        SetWeaponInfo(id, ammo, owner);
    }

    public void AddAmmo(int amount)
    {
        CurrentAmmo += amount;
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
            {
                if (WeaponId == WeaponId.Bazooka)
                    shootAngle = 180 - 35;
                else
                    shootAngle = 180;
            }
            else
            {
                if (WeaponId == WeaponId.Bazooka)
                    shootAngle = 0 + 35;
                else
                    shootAngle = 0;
            }

            bulletScript.SetAngle(shootAngle);
        }

        currentInterval = stat.RapidInterval;

        return bulletScript;
    }

    void SetWeaponInfo(WeaponId id, int ammo, ControllableCharacter inOwner)
    {
        WeaponId = id;
        stat = WeaponDatabase.Instance.GetDefaultWeaponStat(id);
        CurrentAmmo = stat.Ammo;
        owner = inOwner;
    }
}
