using UnityEngine;
using System.Collections;
using RTS;
using System;

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

    bool isInterpolating = false;
    float interpolationTime = NetworkModule.MaxInterpolationTime;
    float accumulatedInterpolationTime = 0f;
    Vector3 interpolationSrc;

    bool isSyncedXStop = false;
    bool isSyncedYStop = false;

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
        charSpriteObject = transform.Find("Sprite").gameObject;

        var playerInfoDisplayGameObject = Instantiate(PlayerInfoDisplay, transform.Find("Sprite"));
        playerInfoDisplayGameObject.transform.localPosition = new Vector3(0, 0.8f, 0);
        playerInfoDisplay = playerInfoDisplayGameObject.GetComponent<PlayerInfoDisplay>();

        gameObject.SetActive(false);
    }

    void Start()
	{
        StartCoroutine("KeepSync");
    }

	// Update is called once per frame
	void Update()
    {
        playerInfoDisplay.SetHP(hp / DefaultHP);

        if (isInterpolating)
        {
            float x = interpolationSrc.x + ((transform.position.x - interpolationSrc.x) * accumulatedInterpolationTime / interpolationTime);
            float y = interpolationSrc.y + ((transform.position.y - interpolationSrc.y) * accumulatedInterpolationTime / interpolationTime);
            float z = interpolationSrc.z + ((transform.position.z - interpolationSrc.z) * accumulatedInterpolationTime / interpolationTime);
            charSpriteObject.transform.position = new Vector3(x, y, z);
            accumulatedInterpolationTime = accumulatedInterpolationTime + Time.deltaTime;
            if (accumulatedInterpolationTime > interpolationTime)
                isInterpolating = false;
        }
        else
        {
            charSpriteObject.transform.localPosition = Vector3.zero;
        }
        
    }

    void FixedUpdate()
    {
        CheckLand();

        if (status == CharacterStatus.Neutral && !isMoving)
            Brake();
    }

        public override void MoveLeft()
    {
        if (!isDead)
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
    }

    public override void MoveRight()
    {
        if (!isDead)
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
        if (!isDead)
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
    }

    public override void ShootWithDamageInfo(DamageInfo info)
    {
        ShootWithDamageInfo(info, gameObject.transform.position);
    }

    public override void ShootWithDamageInfo(DamageInfo info, Vector2 position)
    {
        if (!isDead)
        {
            GameObject bullet = Instantiate(bulletPrefab, gameObject.transform.position, new Quaternion());
            Bullet bulletScript = bullet.GetComponent<Bullet>();

            if (info != null)
            {
                bulletScript.SetInfo(info);
            }

            InvokeEventShoot(CurrentPosition, CurrentVelocity, bulletScript.GetDamageInfo());
        }
    }

    void CheckLand()
    {
        float distToGround = charCollider.bounds.extents.y;
        var checkGrounded = Physics2D.Raycast(transform.position, -Vector2.up, distToGround + (float)0.1);

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

    public override void GetHit(DamageInfo info, float? remainingHp = null)
    {
        if (remainingHp == null)
            hp -= info.Damage;
        else
            hp = (float)remainingHp;

        TestUI.Instance.PrintText("player" + OwnerId + "hp : " + hp + " / Attacker : " + info.AttackerId);

        float radian = Mathf.PI * (float)info.ImpactAngle / 180f;
        float impactX = Mathf.Cos(radian);
        float impactY = Mathf.Sin(radian);
        charRigidbody.AddForce(new Vector2(impactX, impactY) * info.Impact);

        lastAttackedPlayerId = info.AttackerId;

        InvokeEventGetHit(CurrentPosition, CurrentVelocity, info);

        if (IsLocalPlayer && hp <= 0)
        {
            PlayerDie();
        }
    }

    public override void PlayerDie()
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

    public override void MoveWithInterpolation(Vector2 position, Vector2 velocity)
    {
        if (!isDead)
        {
            interpolationTime = Vector2.Distance(transform.position, position) / NetworkModule.InterpolationLongestDistance * NetworkModule.MaxInterpolationTime;
            if (interpolationTime != 0)
            {
                if (interpolationTime > NetworkModule.MaxInterpolationTime)
                    interpolationTime = NetworkModule.MaxInterpolationTime;

                isInterpolating = true;
                accumulatedInterpolationTime = 0;
                interpolationSrc = transform.position;

                //rb2d.position = new Vector2(x, y);
                charRigidbody.MovePosition(position);
                charRigidbody.velocity = velocity;
            }
        }
    }

    public override GameObject GetGameObject()
    {
        return gameObject;
    }

    IEnumerator KeepSync()
    {
        while (true)
        {
            if (!isDead)
            {
                if ((Math.Abs(charRigidbody.velocity.x) > 0.1 || Math.Abs(charRigidbody.velocity.y) > 0.1))
                {
                    InvokeEventSync(charRigidbody.position, charRigidbody.velocity);
                    if (Math.Abs(charRigidbody.velocity.x) > 0.1)
                        isSyncedXStop = false;
                    if (Math.Abs(charRigidbody.velocity.y) > 0.1)
                        isSyncedYStop = false;
                }
                else if((Math.Abs(charRigidbody.velocity.x) == 0 || Math.Abs(charRigidbody.velocity.y) == 0))
                {
                    if (!isSyncedXStop)
                    {
                        InvokeEventSync(charRigidbody.position, charRigidbody.velocity);
                        isSyncedXStop = true;
                    }
                    if (!isSyncedYStop)
                    {
                        InvokeEventSync(charRigidbody.position, charRigidbody.velocity);
                        isSyncedYStop = true;
                    }
                }
            }
            yield return new WaitForSeconds(NetworkModule.SyncFrequency);
        }
    }

}
