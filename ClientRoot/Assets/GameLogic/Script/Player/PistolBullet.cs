using UnityEngine;
using System.Collections;

public class PistolBullet : Bullet
{

    // Use this for initialization
    void Start()
    {
        DamageInfo.Damage = 10;
        DamageInfo.HitRecovery = 10;
        DamageInfo.Impact = 50;
    }

    // Update is called once per frame
    void Update()
    {

    }

    protected override void UpdatePosition()
    {
        if (DamageInfo.shootAngle == 180)
            rb2d.velocity = Vector2.left * bulletSpeed;
        else if (DamageInfo.shootAngle == 0)
            rb2d.velocity = Vector2.right * bulletSpeed;

        Vector2 CurrentPosition = rb2d.position;
        if (Vector2.Distance(StartPosition, CurrentPosition) > range)
            Destroy(gameObject);
    }
}
