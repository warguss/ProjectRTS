using UnityEngine;
using System.Collections;
using RTS;

abstract public class Bullet : MonoBehaviour {

    public int OwnerId { get; set; }
    public  ShootInfo BulletStat { get; set; }

    protected WeaponId weaponId = WeaponId.Pistol;

    protected Vector2 StartPosition;

    protected Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        
    }

    void Awake()
    {
        rb2d = GetComponent<Rigidbody2D>();
        BulletStat = new ShootInfo();
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
        //TestUI.Instance.PrintText("BulletOnTriggerEnter2D");
        if (other.tag == "Player")
        {
            MainCharacter targetPlayer = other.gameObject.GetComponent<MainCharacter>();
            if (targetPlayer.OwnerId != OwnerId)
            {
                if (targetPlayer.IsLocalPlayer)
                {
                    targetPlayer.GetHit(OwnerId, BulletStat);
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

    virtual public void Initialize(int inOwnerId, WeaponStat info)
    {
        StartPosition = rb2d.position;
        weaponId = info.WeaponId;

        BulletStat.BulletRange = info.Range;
        BulletStat.BulletSpeed = info.BulletSpeed;
        BulletStat.HitType = weaponId;
        BulletStat.Damage = info.Damage;
        BulletStat.HitRecovery = info.HitRecovery;
        BulletStat.Impact = info.Impact;
        OwnerId = inOwnerId;
    }

    virtual public void SetAngle(int inAngle)
    {
        BulletStat.shootAngle = inAngle;
    }

    protected abstract void UpdatePosition();
}
