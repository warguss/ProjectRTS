using UnityEngine;
using System.Collections;
using RTS;

public class Bullet : MonoBehaviour {

    public float bulletSpeed;
    public float range;

    public int OwnerId { get; set; }
    public  DamageInfo DamageInfo { get; set; }

    private Vector2 StartPosition;

    private Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        //currently, Damageinfo is constant.
        DamageInfo.Damage = 10;
        DamageInfo.HitRecovery = 10;
        DamageInfo.Impact = 50;

        StartPosition = rb2d.position;
    }

    void Awake()
    {
        rb2d = GetComponent<Rigidbody2D>();
        DamageInfo = new DamageInfo();
    }
	
	// Update is called once per frame
	void Update () {
        
	}

    void FixedUpdate()
    {
        if(DamageInfo.shootAngle == 180)
            rb2d.velocity = Vector2.left * bulletSpeed;
        else if(DamageInfo.shootAngle == 0)
            rb2d.velocity = Vector2.right * bulletSpeed;

        Vector2 CurrentPosition = rb2d.position;
        if(Vector2.Distance(StartPosition, CurrentPosition) > range)
            Destroy(gameObject);
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
                    targetPlayer.GetHit(OwnerId, DamageInfo);
                    //TestUI.Instance.PrintText("BulletOnTriggerEnter2D");
                }

                Destroy(gameObject);
            }
            else
            {
            }
        }
        else if (other.tag == "Wall")
        {
            Destroy(gameObject);
        }
    }
}
