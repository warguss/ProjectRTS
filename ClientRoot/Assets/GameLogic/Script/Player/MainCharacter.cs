using UnityEngine;
using System.Collections;
using RTS;

public enum CharacterStatus
{
	Neutral,
    Attacking,
	Attacked,
}

public class MainCharacter : ControllableCharacter
{
    public float MaxMoveSpeed;
    public float MoveForce;
    public float JumpForce;
    public float DefaultHP;
    public int MaxJumpCount = 2;

    public GameObject bulletPrefab;

    public override void Spawn(Vector2 position)
    {
        gameObject.SetActive(true);
        isDead = false;
        hp = DefaultHP;
        SetLocation(position);

        InvokeEventSpawn(position);
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

        var playerInfoDisplayGameObject = Instantiate(PlayerInfoDisplay, gameObject.transform);
        playerInfoDisplayGameObject.transform.localPosition = new Vector3(0, 0.8f, 0);
        playerInfoDisplay = playerInfoDisplayGameObject.GetComponent<PlayerInfoDisplay>();

        gameObject.SetActive(false);
    }

    void Start()
	{
    }

	// Update is called once per frame
	void Update()
    {
        playerInfoDisplay.SetHP(hp / DefaultHP);
    }

    void FixedUpdate()
    {
        CheckLand();

        if (status == CharacterStatus.Neutral && !isMoving)
            Brake();
    }

        public override void MoveLeft()
    {
        if (!isMoving || !isLeft)
            InvokeEventMove(CurrentPosition, CurrentVelocity, true);

        isMoving = true;
        isLeft = true;

        //rb2d.velocity = new Vector2(-MaxMoveSpeed, rb2d.velocity.y);
        if (-1 * charRigidbody.velocity.x < MaxMoveSpeed)
            charRigidbody.AddForce(Vector2.right * -1 * MoveForce);

        if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
            charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);


    }

    public override void MoveRight()
    {
        if (!isMoving || isLeft)
        {
            InvokeEventMove(CurrentPosition, CurrentVelocity, false);
        }

        isMoving = true;
        isLeft = false;

        //rb2d.velocity = new Vector2(MaxMoveSpeed, rb2d.velocity.y);
        if (1 * charRigidbody.velocity.x < MaxMoveSpeed)
            charRigidbody.AddForce(Vector2.right * 1 * MoveForce);

        if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
            charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);
    }

    public override void MoveStop()
    {
        if(status == CharacterStatus.Neutral)
        {
            if (isMoving)
                InvokeEventStop(CurrentPosition, CurrentVelocity);

            isMoving = false;

            status = CharacterStatus.Neutral;
        }
    }

    void Brake()
    {
        charRigidbody.velocity = new Vector2(charRigidbody.velocity.x * (float)0.8, charRigidbody.velocity.y);
    }

    public override void InitialSync()
    {
        if (!isDead)
        {
            InvokeEventSpawn(CurrentPosition);
            if (isMoving)
            {
                if (isLeft)
                    InvokeEventMove(CurrentPosition, CurrentVelocity, true);
                else
                    InvokeEventMove(CurrentPosition, CurrentVelocity, false);
            }
        }
    }

    public override void Jump()
    {
        if (jumpCount < MaxJumpCount)
        {
            jumpCount++;

            //rb2d.AddForce(Vector2.up*JumpForce);
            charRigidbody.velocity = new Vector2(charRigidbody.velocity.x, JumpForce);

            InvokeEventJump(CurrentPosition, CurrentVelocity);
        }
    }

    public override void Shoot()
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

    public override void ShootWithDamageInfo(DamageInfo info)
    {
        ShootWithDamageInfo(info, gameObject.transform.position);
    }

    public override void ShootWithDamageInfo(DamageInfo info, Vector2 position)
    {
        GameObject bullet = Instantiate(bulletPrefab, gameObject.transform.position, new Quaternion());
        Bullet bulletScript = bullet.GetComponent<Bullet>();

        if (info != null)
        {
            bulletScript.SetInfo(info);
        }

        InvokeEventShoot(CurrentPosition, CurrentVelocity, bulletScript.GetDamageInfo());
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

    public override void GetHit(DamageInfo info)
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

            InvokeEventGetHit(CurrentPosition, CurrentVelocity, info);

            if (hp <= 0)
            {
                Dead();
            }
        }
    }

    public override void Dead()
    {
        if (!isDead)
        {
            isDead = true;
            InvokeEventDead(CurrentPosition, lastAttackedPlayerId);
            gameObject.SetActive(false);
        }
    }

    public override void SetLocation(Vector2 position)
    {
        if (!isDead)
        {
            gameObject.transform.position = position;
            //charRigidbody.MovePosition(position);
        }
    }

    public override void MoveTo(Vector2 position, Vector2 velocity)
    {
        if (!isDead)
        {
            //rb2d.position = new Vector2(x, y);
            charRigidbody.MovePosition(position);
            charRigidbody.velocity = velocity;
        }
    }

    public override GameObject GetGameObject()
    {
        return gameObject;
    }
    //void Networkinterpolation(float posX, float posY, float speedX, float speedY, CharacterStatus status)
    //{
    //}
}
