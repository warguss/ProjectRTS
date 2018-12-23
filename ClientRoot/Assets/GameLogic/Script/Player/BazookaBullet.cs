using UnityEngine;
using System.Collections;

public class BazookaBullet : Bullet
{
    private const int parabolaGravity = 1;

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
        TestUI.Instance.PrintText("BulletOnTriggerEnter2D");
        if (other.tag == "Player")
        {
            MainCharacter targetPlayer = other.gameObject.GetComponent<MainCharacter>();
            if (targetPlayer.OwnerId != OwnerId)
            {
                if (targetPlayer.IsLocalPlayer)
                {
                    //폭발 생성하기
                    TestUI.Instance.PrintText("BazookaBulletOnTriggerEnter2D");
                }

                Destroy(gameObject);
            }
            else
            {
            }
        }
        else if (other.tag == "Wall")
        {
            //폭발 생성하기
            Destroy(gameObject);
        }
    }
}
