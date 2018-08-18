using UnityEngine;
using System.Collections;
using RTS;

abstract public class Bullet : MonoBehaviour {

    public float bulletSpeed;
    public float range;

    public int OwnerId { get; set; }
    public  DamageInfo DamageInfo { get; set; }

    protected Vector2 StartPosition;

    protected Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        //currently, Damageinfo is constant.
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

    protected abstract void UpdatePosition();
}
