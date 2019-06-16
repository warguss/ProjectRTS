using UnityEngine;
using System.Collections;

public class BazookaBullet : Bullet
{
    public GameObject ExplosionPrefab;

    private const float parabolaGravity = 0.75f;

    // Use this for initialization
    void Start()
    {
        
    }

    // Update is called once per frame
    void Update()
    {

    }

    protected override void UpdatePosition(float elapsedTime)
    {
        float radian = Mathf.PI * (float)BulletStat.ShootAngle / 180f;
        float impactX = Mathf.Cos(radian);
        float impactY = Mathf.Sin(radian);

        impactY = impactY - parabolaGravity * elapsedTime;

        //rb2d.velocity = new Vector2(impactX, impactY) * BulletStat.BulletSpeed;
        //Vector2 CurrentPosition = rb2d.position;

        Vector2 NewPosition = new Vector2(StartPosition.x + impactX * BulletStat.BulletSpeed * elapsedTime,
                                          StartPosition.y + impactY * BulletStat.BulletSpeed * elapsedTime);
        rb2d.position = NewPosition;

        if (Vector2.Distance(StartPosition, NewPosition) > BulletStat.BulletRange)
            Destroy(gameObject);
    }

    override public void SetAngle(int inAngle)
    {
        BulletStat.ShootAngle = inAngle;
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.tag == "Player")
        {
            MainCharacter targetPlayer = other.gameObject.GetComponent<MainCharacter>();
            if (targetPlayer.OwnerId != OwnerId)
            {
                if (targetPlayer.IsLocalPlayer)
                {
                    CreateExplotion();
                }

                Destroy(gameObject);
            }
            else
            {
            }
        }
        else if (other.tag == "Wall")
        {
            CreateExplotion();
            Destroy(gameObject);
        }
    }

    void CreateExplotion()
    {
        var Explosion = Instantiate(ExplosionPrefab, new Vector2(rb2d.position.x, rb2d.position.y), new Quaternion());
        ExplosionDamage explosionScript = Explosion.GetComponent<ExplosionDamage>();
        explosionScript.Initialize(WeaponId.Bazooka, BulletStat.Damage, 0.8f, 1.5f, BulletStat.ImpactScale, OwnerId);
        explosionScript.DoExplotion();
    }
}
