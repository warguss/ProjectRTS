using UnityEngine;
using System.Collections;
using RTS;

public enum CharacterStatus
{
	Neutral,
    Moving,
    Attacking,
	Attacked,
}

public class MainCharacter : MonoBehaviour
{
    public float MaxMoveSpeed;
    public float MoveForce;
    public float JumpForce;
    public int airJumpCount = 1;

    public GameObject bulletPrefab;

    private Rigidbody2D rb2d;
     
    public int playerId{ get; set; }

    private CharacterStatus status = CharacterStatus.Neutral;
    private int hitRecovery = 0;

    private float hp = 100;

    private bool inAir = false;
    private bool isLeft = true;

    private PlayerInput currentInput;

    public Vector2 CurrentPosition
    {
        get
        {
            return rb2d.position;
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
		rb2d = GetComponent<Rigidbody2D>();
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
        if (currentInput.jump && (status == CharacterStatus.Neutral || status == CharacterStatus.Moving))
            Jump();

        if (currentInput.fire && (status == CharacterStatus.Neutral || status == CharacterStatus.Moving))
            Shoot();
        
        if (currentInput.left && status != CharacterStatus.Attacked)
            MoveLeft();
        
        else if (currentInput.right && status != CharacterStatus.Attacked)
            MoveRight();
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
                currentInput.left = true;
                MoveEvent(true);
                break;
            case PlayerAction.Right:
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
        if (-1 * rb2d.velocity.x < MaxMoveSpeed)
            rb2d.AddForce(Vector2.right * -1 * MoveForce);
        
        if (Mathf.Abs (rb2d.velocity.x) > MaxMoveSpeed)
            rb2d.velocity = new Vector2(Mathf.Sign (rb2d.velocity.x) * MaxMoveSpeed, rb2d.velocity.y);
    }

    void MoveRight()
    {
        isLeft = false;
        if (1 * rb2d.velocity.x < MaxMoveSpeed)
            rb2d.AddForce(Vector2.right * 1 * MoveForce);

        if (Mathf.Abs (rb2d.velocity.x) > MaxMoveSpeed)
            rb2d.velocity = new Vector2(Mathf.Sign (rb2d.velocity.x) * MaxMoveSpeed, rb2d.velocity.y);
    }

    void Jump()
    {
        //rb2d.AddForce(Vector2.up*JumpForce);
        rb2d.velocity = new Vector2(rb2d.velocity.x, JumpForce);
        currentInput.jump = false;

        JumpEvent();
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

    public void GetHit(int damage, int hitRecovery, int impact, int impactAngle)
    {
        hp -= damage;
        Debug.Log("player" + playerId + "hp : " + hp);

        float radian = Mathf.PI * (float)impactAngle / 180f;
        float impactX = Mathf.Cos(radian);
        float impactY = Mathf.Sin(radian);
        rb2d.AddForce(new Vector2(impactX, impactY) * impact);

        GetHitEvent(damage, hitRecovery, impact, impactAngle);
    }

    public void Dead()
    {
        Destroy(gameObject);
    }

    public void MoveTo(float x, float y)
    {
        rb2d.position = new Vector2(x, y);
    }

    //void Networkinterpolation(float posX, float posY, float speedX, float speedY, CharacterStatus status)
    //{
    //}
}
