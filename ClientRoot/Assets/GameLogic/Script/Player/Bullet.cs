using UnityEngine;
using System.Collections;
using RTS;

public class Bullet : MonoBehaviour {

    public float bulletSpeed;
    public int range;

    private int angle = 0;
    private int OwnerPlayer = 0;

    private PosInt StartPosition;

    private Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        rb2d = GetComponent<Rigidbody2D>();
        StartPosition = Utils.RbPosToGamePos(rb2d.position);
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

        PosInt CurrentPosition = Utils.RbPosToGamePos(rb2d.position);
        if(Utils.GamePosDistance(StartPosition, CurrentPosition) > range)
            Destroy(gameObject);
    }

    void OnTriggerEnter2D(Collider2D other)
    {
        if (other.tag == "Player")
        {
            MainCharacter targetPlayer = other.gameObject.GetComponent<MainCharacter>();
            if (targetPlayer.playerId != OwnerPlayer)
            {
                targetPlayer.GetHit(10, 10, 50, angle);
                Destroy(gameObject);
            }
        }
        else if (other.tag == "Wall")
        {
            Destroy(gameObject);
        }
    }

    public void SetAngle(int inAngle)
    {
        angle = inAngle;
    }

    public void SetOwner(int player)
    {
        OwnerPlayer = player;
    }
}
