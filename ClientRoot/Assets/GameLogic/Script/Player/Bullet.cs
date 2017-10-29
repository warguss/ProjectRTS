using UnityEngine;
using System.Collections;

public class Bullet : MonoBehaviour {

    public float bulletSpeed;
    public int angle = 0;
    public int OwnerPlayer = 0;

    private Rigidbody2D rb2d;

	// Use this for initialization
	void Start () {
        rb2d = GetComponent<Rigidbody2D>();
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
}
