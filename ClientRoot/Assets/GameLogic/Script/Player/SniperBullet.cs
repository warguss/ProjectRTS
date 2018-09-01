using UnityEngine;
using System.Collections;

public class SniperBullet : Bullet
{
    // Use this for initialization
    void Start()
    {
        weaponId = WeaponId.Sniper;
    }

    // Update is called once per frame
    void Update()
    {

    }

    protected override void UpdatePosition()
    {
        float radian = Mathf.PI * (float)DamageInfo.shootAngle / 180f;
        float impactX = Mathf.Cos(radian);
        float impactY = Mathf.Sin(radian);

        rb2d.velocity = new Vector2(impactX, impactY) * bulletSpeed;

        Vector2 CurrentPosition = rb2d.position;
        if (Vector2.Distance(StartPosition, CurrentPosition) > range)
            Destroy(gameObject);
    }

    override public void SetAngle(int inAngle)
    {
        DamageInfo.shootAngle = inAngle;
        DamageInfo.ImpactAngle = inAngle;
    }
}
