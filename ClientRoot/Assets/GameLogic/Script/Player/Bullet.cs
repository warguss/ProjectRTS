using UnityEngine;
using System.Collections;
using RTS;

public class Bullet : MonoBehaviour {

    public float bulletSpeed;
    public float range;

    private int angle = 0;
    private int ownerPlayer = 0;

    private DamageInfo damageInfo; 

    private Vector2 StartPosition;

    private Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        rb2d = GetComponent<Rigidbody2D>();
        damageInfo = new DamageInfo();

        //currently, Damageinfo is constant.
        damageInfo.Damage = 10;
        damageInfo.HitRecovery = 10;
        damageInfo.Impact = 50;

        StartPosition = rb2d.position;

    }
	
	// Update is called once per frame
	void Update () {
        
	}

    void FixedUpdate()
    {
        if(angle == 180)
            rb2d.velocity = Vector2.left * bulletSpeed;
        else if(angle == 0)
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
            if (targetPlayer.OwnerId != ownerPlayer)
            {
                //if (targetPlayer.OwnerId == GameLogic.Instance.myId)
                targetPlayer.GetHit(damageInfo);

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

    public DamageInfo GetDamageInfo()
    {
        return damageInfo;
    }

    public void SetInfo(int ownerId, int inAngle)
    {
        angle = inAngle;
        damageInfo.shootAngle = inAngle;

        ownerPlayer = ownerId;
        damageInfo.AttackerId = ownerId;
    }

    public void SetInfo(DamageInfo info)
    {
        damageInfo = info;
    }
}
