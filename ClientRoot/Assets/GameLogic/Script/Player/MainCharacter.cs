using UnityEngine;
using System.Collections;

public enum CharacterStatus
{
	Neutral,
    Moving,
    inAir,
    Attacking,
	Attacked,
}

public class MainCharacter : MonoBehaviour
{
	public float MaxMoveSpeed = 50;
	public float MoveForce = 50;
	public float JumpForce = 1000;

    public GameObject bulletPrefab;

    private Rigidbody2D rb2d;

    private CharacterStatus status = CharacterStatus.Neutral;
    private int hitRecovery = 0;

    //private int posX;
    //private int posY;
    //private int speedX;
    //private int speedY;

	// Use this for initialization
	void Start()
	{
		rb2d = GetComponent<Rigidbody2D>();
	}
	
	// Update is called once per frame
	void Update()
    {

	}

	void FixedUpdate()
	{
        if (Input.GetButtonDown("Jump") && (status == CharacterStatus.Neutral || status == CharacterStatus.Moving))
            Jump();

        if (Input.GetButtonDown("Fire") && (status == CharacterStatus.Neutral || status == CharacterStatus.Moving))
            Fire();
        
        if (Input.GetButton("Left") && status != CharacterStatus.Attacked)
            MoveLeft();
        else if (Input.GetButton("Right") && status != CharacterStatus.Attacked)
            MoveRight();
	}

    void MoveLeft()
    {
        if (-1 * rb2d.velocity.x < MaxMoveSpeed)
            rb2d.AddForce(Vector2.right * -1 * MoveForce);
        
        if (Mathf.Abs (rb2d.velocity.x) > MaxMoveSpeed)
            rb2d.velocity = new Vector2(Mathf.Sign (rb2d.velocity.x) * MaxMoveSpeed, rb2d.velocity.y);
    }

    void MoveRight()
    {
        if (1 * rb2d.velocity.x < MaxMoveSpeed)
            rb2d.AddForce(Vector2.right * 1 * MoveForce);

        if (Mathf.Abs (rb2d.velocity.x) > MaxMoveSpeed)
            rb2d.velocity = new Vector2(Mathf.Sign (rb2d.velocity.x) * MaxMoveSpeed, rb2d.velocity.y);
    }

    void Stop()
    {
        
    }

    void Jump()
    {
        rb2d.AddForce(new Vector2(0f, JumpForce));
    }

    void Fire()
    {
        Instantiate(bulletPrefab, gameObject.transform.position, new Quaternion());
    }

    void GetHit(int damage, int hitRecovery)
    {
        
    }

    //void Networkinterpolation(float posX, float posY, float speedX, float speedY, CharacterStatus status)
    //{
    //}
}
