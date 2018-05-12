using UnityEngine;
using System.Collections;
using RTS;

public enum CharacterStatus
{
	Neutral,
    Attacking,
	Attacked,
}

public class MainCharacter : MonoBehaviour, IControllableCharacter
{
    public int OwnerId { get; set; }
    public string OwnerName { get; set; }

    public float MaxMoveSpeed;
    public float MoveForce;
    public float JumpForce;
    public float DefaultHP;
    public int MaxJumpCount = 2;

    public GameObject bulletPrefab;

    private Rigidbody2D charRigidbody;
    private Collider2D charCollider;

    private CharacterStatus status = CharacterStatus.Neutral;
    private int hitRecovery = 0;

    private float hp;
    private int jumpCount = 0;
    private int lastAttackedPlayerId = -1;

    private bool isGrounded = true;
    private bool isLeft = true;
    private bool isMoving = false;
    private bool isDead = false;

    public event CharEventMove MoveEvent;
    public event CharEventStop StopEvent;
    public event CharEventJump JumpEvent;
    public event CharEventGetHit GetHitEvent;
    public event CharEventShoot ShootEvent;
    public event CharEventSpawn SpawnEvent;
    public event CharEventDead DeadEvent;

    public void SetOwner(int owner)
    {
        OwnerId = owner;
    }

    public void SetName(string name)
    {
        OwnerName = name;
    }

    public void Spawn(Vector2 position)
    {
        gameObject.SetActive(true);
        isDead = false;
        hp = DefaultHP;
        SetLocation(position);

        SpawnEvent?.Invoke(position);
    }

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

    // Use this for initialization
    void Awake()
    {
        isDead = true;
        charRigidbody = GetComponent<Rigidbody2D>();
        charCollider = GetComponent<Collider2D>();
        gameObject.SetActive(false);
    }

    void Start()
	{
    }

	// Update is called once per frame
	void Update()
    {
        //Debug.Log(posX + ", " + posY);
	}

    void FixedUpdate()
    {
        CheckLand();

        if (status == CharacterStatus.Neutral && !isMoving)
            Brake();
    }

        public void MoveLeft()
    {
        if (!isMoving || !isLeft)
            MoveEvent?.Invoke(CurrentPosition, CurrentVelocity, true);

        isMoving = true;
        isLeft = true;

        //rb2d.velocity = new Vector2(-MaxMoveSpeed, rb2d.velocity.y);
        if (-1 * charRigidbody.velocity.x < MaxMoveSpeed)
            charRigidbody.AddForce(Vector2.right * -1 * MoveForce);

        if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
            charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);


    }

    public void MoveRight()
    {
        if (!isMoving || isLeft)
        {
            MoveEvent?.Invoke(CurrentPosition, CurrentVelocity, false);
        }

        isMoving = true;
        isLeft = false;

        //rb2d.velocity = new Vector2(MaxMoveSpeed, rb2d.velocity.y);
        if (1 * charRigidbody.velocity.x < MaxMoveSpeed)
            charRigidbody.AddForce(Vector2.right * 1 * MoveForce);

        if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
            charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);
    }

    public void MoveStop()
    {
        if(status == CharacterStatus.Neutral)
        {
            if (isMoving)
                StopEvent?.Invoke(CurrentPosition, CurrentVelocity);

            isMoving = false;

            status = CharacterStatus.Neutral;
        }
    }

    void Brake()
    {
        charRigidbody.velocity = new Vector2(charRigidbody.velocity.x * (float)0.8, charRigidbody.velocity.y);
    }

    public void InitialSync()
    {
        if (!isDead)
        {
            SpawnEvent?.Invoke(CurrentPosition);
            if (isMoving)
            {
                if (isLeft)
                    MoveEvent?.Invoke(CurrentPosition, CurrentVelocity, true);
                else
                    MoveEvent?.Invoke(CurrentPosition, CurrentVelocity, false);
            }
        }
    }

    public void Jump()
    {
        if (jumpCount < MaxJumpCount)
        {
            jumpCount++;

            //rb2d.AddForce(Vector2.up*JumpForce);
            charRigidbody.velocity = new Vector2(charRigidbody.velocity.x, JumpForce);

            JumpEvent?.Invoke(CurrentPosition, CurrentVelocity);
        }
    }

    public void Shoot()
    {
        int shootAngle;

        if (isLeft)
            shootAngle = 180;
        else
            shootAngle = 0;

        DamageInfo damageInfo = new DamageInfo
        {
            AttackerId = OwnerId,
            shootAngle = shootAngle,
            ImpactAngle = shootAngle,
            Impact = 50,
            Damage = 10,
        };

        ShootWithDamageInfo(damageInfo);
    }

    public void ShootWithDamageInfo(DamageInfo info)
    {
        ShootWithDamageInfo(info, gameObject.transform.position);
    }

    public void ShootWithDamageInfo(DamageInfo info, Vector2 position)
    {
        GameObject bullet = Instantiate(bulletPrefab, gameObject.transform.position, new Quaternion());
        Bullet bulletScript = bullet.GetComponent<Bullet>();

        if (info != null)
        {
            bulletScript.SetInfo(info);
        }

        ShootEvent?.Invoke(CurrentPosition, CurrentVelocity, bulletScript.GetDamageInfo());
    }

    void CheckLand()
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

    public void GetHit(DamageInfo info)
    {
        if (!GameLogic.Instance.isOnline || OwnerId == GameLogic.Instance.myId)
        {
            hp -= info.Damage;
            TestUI.Instance.PrintText("player" + OwnerId + "hp : " + hp + " / Attacker : " + info.AttackerId);

            float radian = Mathf.PI * (float)info.ImpactAngle / 180f;
            float impactX = Mathf.Cos(radian);
            float impactY = Mathf.Sin(radian);
            charRigidbody.AddForce(new Vector2(impactX, impactY) * info.Impact);

            lastAttackedPlayerId = info.AttackerId;

            GetHitEvent?.Invoke(CurrentPosition, CurrentVelocity, info);

            if (hp <= 0)
            {
                Dead();
            }
        }
    }

    public void Dead()
    {
        if (!isDead)
        {
            isDead = true;
            DeadEvent?.Invoke(CurrentPosition, lastAttackedPlayerId);
            gameObject.SetActive(false);
        }
    }

    public void SetLocation(Vector2 position)
    {
        if (!isDead)
        {
            gameObject.transform.position = position;
            //charRigidbody.MovePosition(position);
        }
    }

    public void MoveTo(Vector2 position, Vector2 velocity)
    {
        if (!isDead)
        {
            //rb2d.position = new Vector2(x, y);
            charRigidbody.MovePosition(position);
            charRigidbody.velocity = velocity;
        }
    }

    public GameObject GetGameObject()
    {
        return gameObject;
    }
    //void Networkinterpolation(float posX, float posY, float speedX, float speedY, CharacterStatus status)
    //{
    //}
}
