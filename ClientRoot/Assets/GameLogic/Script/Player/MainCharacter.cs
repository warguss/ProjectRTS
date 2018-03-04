using UnityEngine;
using System.Collections;
using RTS;

public enum CharacterStatus
{
	Neutral,
    Attacking,
	Attacked,
}

public class MainCharacter : MonoBehaviour
{
    public float MaxMoveSpeed;
    public float MoveForce;
    public float JumpForce;
    public int MaxJumpCount = 2;

    public GameObject bulletPrefab;

    private Rigidbody2D charRigidbody;
    private Collider2D charCollider;
     
    public int playerId{ get; set; }

    private CharacterStatus status = CharacterStatus.Neutral;
    private int hitRecovery = 0;

    private float hp = 100;
    private int jumpCount = 0;

    private bool isGrounded = true;
    private bool isLeft = true;

    private PlayerInput currentInput;

    public Vector2 CurrentPosition
    {
        get
        {
            return charRigidbody.position;
        }
    }

    public Vector2 CurrentVelocity
    {
        get
        {
            return charRigidbody.velocity;
        }
    }

    public delegate void PlayerEventMove(bool isLeft);
    public delegate void PlayerEventStop();
    public delegate void PlayerEventJump();
    public delegate void PlayerEventGetHit(int damage, int hitRecovery, int impact, int impactAngle);
    public delegate void PlayerEventShoot(bool isLeft);

    public event PlayerEventMove MoveEvent;
    public event PlayerEventStop StopEvent;
    public event PlayerEventJump JumpEvent;
    public event PlayerEventGetHit GetHitEvent;
    public event PlayerEventShoot ShootEvent;

    // Use this for initialization
    void Start()
	{
		charRigidbody = GetComponent<Rigidbody2D>();
        charCollider = GetComponent<Collider2D>();

    }

	// Update is called once per frame
	void Update()
    {
        //Debug.Log(posX + ", " + posY);
        if (hp <= 0)
        {
            Dead();
        }
	}

	void FixedUpdate()
	{
        if (currentInput.jump && (status == CharacterStatus.Neutral))
            Jump();

        if (currentInput.fire && (status == CharacterStatus.Neutral))
            Shoot();

        if (currentInput.left && status != CharacterStatus.Attacked)
            MoveLeft();
        else if (currentInput.right && status != CharacterStatus.Attacked)
            MoveRight();
        else if (!currentInput.left && !currentInput.right)
            MoveStop();

        checkLand();
	}

    public void SetInput(PlayerAction action)
    {
        switch (action)
        {
            case PlayerAction.Stop:
                currentInput.left = false;
                currentInput.right = false;
                StopEvent();
                break;
            case PlayerAction.Left:
                currentInput.right = false;
                currentInput.left = true;
                MoveEvent(true);
                break;
            case PlayerAction.Right:
                currentInput.left = false;
                currentInput.right = true;
                MoveEvent(false);
                break;
            case PlayerAction.Jump:
                currentInput.jump = true;
                break;
            case PlayerAction.Fire:
                currentInput.fire = true;
                break;
        }
    }

    void MoveLeft()
    {
        isLeft = true;

        //rb2d.velocity = new Vector2(-MaxMoveSpeed, rb2d.velocity.y);
        if (-1 * charRigidbody.velocity.x < MaxMoveSpeed)
            charRigidbody.AddForce(Vector2.right * -1 * MoveForce);

        if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
            charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);
    }

    void MoveRight()
    {
        isLeft = false;

        //rb2d.velocity = new Vector2(MaxMoveSpeed, rb2d.velocity.y);
        if (1 * charRigidbody.velocity.x < MaxMoveSpeed)
            charRigidbody.AddForce(Vector2.right * 1 * MoveForce);

        if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
            charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);
    }

    void MoveStop()
    {
        if(status == CharacterStatus.Neutral)
        {
            charRigidbody.velocity = new Vector2(charRigidbody.velocity.x * (float)0.8, charRigidbody.velocity.y);

            status = CharacterStatus.Neutral;
        }
    }

    void Jump()
    {
        currentInput.jump = false;
        if (jumpCount < MaxJumpCount)
        {
            jumpCount++;

            //rb2d.AddForce(Vector2.up*JumpForce);
            charRigidbody.velocity = new Vector2(charRigidbody.velocity.x, JumpForce);

            JumpEvent();
        }
    }

    void Shoot()
    {
        GameObject bullet = (GameObject)Instantiate(bulletPrefab, gameObject.transform.position, new Quaternion());
        Bullet bulletScript = bullet.GetComponent<Bullet>();

        bulletScript.SetOwner(playerId);

        if (isLeft)
            bulletScript.SetAngle(180);
        else
            bulletScript.SetAngle(0);

        currentInput.fire = false;

        ShootEvent(isLeft);
    }

    void checkLand()
    {
        float distToGround = charCollider.bounds.extents.y;
        bool checkGrounded = Physics2D.Raycast(transform.position, -Vector2.up, distToGround + (float)0.1, 1 << LayerMask.NameToLayer("Wall"));

        if (checkGrounded)
        {
            if (isGrounded)
            {

            }
            else
            {
                Land();
            }
        }
        else
        {
            if (isGrounded)
            {
                isGrounded = false;
            }
            else
            {

            }
        }
    }

    void Land()
    {
        isGrounded = true;
        jumpCount = 0;
    }

    public void GetHit(int damage, int hitRecovery, int impact, int impactAngle)
    {
        hp -= damage;
        Debug.Log("player" + playerId + "hp : " + hp);

        float radian = Mathf.PI * (float)impactAngle / 180f;
        float impactX = Mathf.Cos(radian);
        float impactY = Mathf.Sin(radian);
        charRigidbody.AddForce(new Vector2(impactX, impactY) * impact);

        GetHitEvent(damage, hitRecovery, impact, impactAngle);
    }

    public void Dead()
    {
        Destroy(gameObject);
    }

    public void LeaveGame()
    {
        Destroy(gameObject);
    }

    public void MoveTo(float x, float y)
    {
        //rb2d.position = new Vector2(x, y);
        charRigidbody.MovePosition(new Vector2(x, y));
    }

    public void MoveTo(float x, float y, float velocityX, float velocityY)
    {
        //rb2d.position = new Vector2(x, y);
        charRigidbody.MovePosition(new Vector2(x, y));
        charRigidbody.velocity = new Vector2(velocityX, velocityY);
    }

    //void Networkinterpolation(float posX, float posY, float speedX, float speedY, CharacterStatus status)
    //{
    //}
}
