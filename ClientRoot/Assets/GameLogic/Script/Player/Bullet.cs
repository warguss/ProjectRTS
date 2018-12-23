using UnityEngine;
using System.Collections;
using RTS;

abstract public class Bullet : MonoBehaviour {

    public int OwnerId { get; set; }
    public  ShootInfo BulletStat { get; set; }

    protected WeaponId weaponId = WeaponId.Pistol;

    protected Vector2 StartPosition;

    protected Rigidbody2D rb2d;

    protected float startTime = 0f;
    protected float elapsedTime = 0f;

	// Use this for initialization
	void Start () {
        
    }

    void Awake()
    {
        rb2d = GetComponent<Rigidbody2D>();
        BulletStat = new ShootInfo();
    }
	
	// Update is called once per frame
	void Update ()
    {
        
	}

    void FixedUpdate()
    {
        elapsedTime = Time.time - startTime;
        UpdatePosition(elapsedTime);
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
                    float radian = Mathf.PI * BulletStat.ShootAngle / 180f;

                    HitInfo hitInfo = new HitInfo();
                    hitInfo.Damage = BulletStat.Damage;
                    hitInfo.HitType = weaponId;
                    hitInfo.ImpactX = Mathf.Cos(radian) * BulletStat.ImpactScale;
                    hitInfo.ImpactY = Mathf.Sin(radian) * BulletStat.ImpactScale; 

                    targetPlayer.GetHit(OwnerId, hitInfo);
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
        BulletStat.ShootType = weaponId;
        BulletStat.Damage = info.Damage;
        BulletStat.ImpactScale = info.ImpactScale;
        OwnerId = inOwnerId;

        startTime = Time.time;
    }

    virtual public void SetAngle(int inAngle)
    {
        BulletStat.ShootAngle = inAngle;
    }

    protected abstract void UpdatePosition(float elapsedTime);
}
