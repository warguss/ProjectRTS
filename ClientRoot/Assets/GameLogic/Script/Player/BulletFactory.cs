﻿using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class BulletFactory : MonoBehaviour {

    static public BulletFactory Instance;

    public GameObject PistolBulletPrefab;
    public GameObject SniperBulletPrefab;

    private void Start()
    {
        Instance = this;
    }

    public GameObject InstantiateBullet(WeaponId weaponId)
    {
        GameObject returnObject;

        switch(weaponId)
        {
            case WeaponId.Pistol:
                returnObject = Instantiate(PistolBulletPrefab);
                break;

            case WeaponId.Sniper:
                returnObject = Instantiate(SniperBulletPrefab);
                break;

            default:
                returnObject = Instantiate(PistolBulletPrefab);
                break;
        }

        return returnObject;
    }
}