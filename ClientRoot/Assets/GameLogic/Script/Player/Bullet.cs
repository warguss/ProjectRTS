using UnityEngine;
using System.Collections;
using RTS;

abstract public class Bullet : MonoBehaviour {

    public int OwnerId { get; set; }
    public  DamageInfo DamageInfo { get; set; }

    protected WeaponId weaponId;
    protected float bulletSpeed;
    protected float range;

    protected Vector2 StartPosition;

    protected Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        
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
        UpdatePosition();
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

    public void Initialize(int inOwnerId, WeaponStat info)
    {
        StartPosition = rb2d.position;
        bulletSpeed = info.BulletSpeed;
        range = info.Range;
        DamageInfo.Damage = info.Damage;
        DamageInfo.HitRecovery = info.HitRecovery;
        DamageInfo.Impact = info.Impact;
        OwnerId = inOwnerId;
    }

    virtual public void SetAngle(int inAngle)
    {
        DamageInfo.shootAngle = inAngle;
        DamageInfo.ImpactAngle = inAngle;///////////////////
    }

    protected abstract void UpdatePosition();
}
