using UnityEngine;
using System.Collections;
using RTS;
using System;
using System.Collections.Generic;

public class MainCharacter : ControllableCharacter
{
    const float DEFAULT_HP_RECOVER_AMOUNT = 30;
    const float DEFAULT_HP = 100;

    public float MaxMoveSpeed;
    public float MoveForce;
    public float JumpForce;
    public int MaxJumpCount = 2;

    bool isInterpolating = false;
    float interpolationTime = NetworkModule.MaxInterpolationTime;
    float accumulatedInterpolationTime = 0f;
    Vector3 interpolationSrc;

    bool isSyncedXStop = false;
    bool isSyncedYStop = false;

    public override void Spawn(Vector2 position)
    {
        gameObject.SetActive(true);
        StartCoroutine("KeepSync");
        isDead = false;
        state.hp = state.MaxHP;
        SetLocation(position);

        Inventory.ClearItem();
        Inventory.AddItem(WeaponId.Pistol);////////////////////
        Inventory.AddItem(WeaponId.Sniper);////////////////////
        Inventory.AddItem(WeaponId.Bazooka);////////////////////

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
        SpriteOverlay = transform.Find("SpriteOverlay").GetComponent<SpriteOverlayScript>();

        Inventory = new PlayerInventory(this);
        //Inventory.AddItem(WeaponId.Pistol);////////////////////
        //Inventory.AddItem(WeaponId.Sniper);////////////////////

        state = new CharacterStateInfo();

        var playerInfoDisplayGameObject = Instantiate(PlayerInfoDisplay, transform.Find("Sprite"));
        playerInfoDisplayGameObject.transform.localPosition = new Vector3(0, 0.8f, 0);
        playerInfoDisplay = playerInfoDisplayGameObject.GetComponent<PlayerInfoDisplay>();

        state.MaxHP = DEFAULT_HP;

        gameObject.SetActive(false);
        StopCoroutine("KeepSync");
    }

    void Start()
	{
        
    }

	// Update is called once per frame
	void Update()
    {
        playerInfoDisplay.SetHP(state.hp / state.MaxHP);
        Inventory.UpdateWeaponInterval(Time.deltaTime);
        state.UpdateStateInfo(Time.deltaTime);
        
        if(state.Invincible)
        {
            SpriteOverlay.SetInvincible(true);
        }
        else
        {
            SpriteOverlay.SetInvincible(false);
        }

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

        if (state.CurrentState == CharacterState.Neutral && !isMoving)
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
        if(state.CurrentState == CharacterState.Neutral)
        {
            if (isMoving)
                InvokeEventStop(CurrentPosition, CurrentVelocity);

            isMoving = false;

            state.CurrentState = CharacterState.Neutral;
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
            //ShootInfo shootInfo = new ShootInfo
            //{
            //    shootAngle = shootAngle,
            //    ImpactAngle = shootAngle,
            //    Impact = 50,
            //    Damage = 10,
            //};

            ShootWithShootInfo(null);
        }
    }

    public override void ShootWithShootInfo(ShootInfo info)
    {
        ShootWithShootInfo(info, gameObject.transform.position);
    }

    public override void ShootWithShootInfo(ShootInfo info, Vector2 position)
    {
        if (!isDead)
        {
            PlayerWeapon currentWeapon = Inventory.GetCurrentWeapon();
            if (currentWeapon != null)
            {
                if (currentWeapon.IsShootable() || !IsLocalPlayer)
                {
                    var bullet = currentWeapon.Shoot(info, position);

                    InvokeEventShoot(CurrentPosition, CurrentVelocity, bullet.BulletStat, currentWeapon.WeaponId);
                    Inventory.InvokeSetAmmoEvent(currentWeapon.WeaponId, currentWeapon.CurrentAmmo);

                    if (currentWeapon.CurrentAmmo == 0 && IsLocalPlayer) // 리모트 유저의 남은 탄환은 동기화가 안 되서 덜 썼는데도 무기가 지워지는 현상 발생. IsLocalPlayer 체크 추가.
                    {
                        Inventory.DeleteItem(currentWeapon.WeaponId);
                    }
                }
            }
        }
    }

    public override void ChangeWeapon(WeaponId inWeaponId)
    {
        Inventory.ChangeWeapon(inWeaponId);
        state.CurrentWeapon = inWeaponId;
        InvokeEventChangeWeapon(CurrentPosition, CurrentVelocity, inWeaponId);
    }

    public override void ChangeToNextWeapon() // 확인 필요. GameLogic에서 Inventory의 Change Weapon을 직접 호출중?
    {
        Inventory.ChangeToNextWeapon();
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

    public override void GetHit(int attackerId, HitInfo info, float? remainingHp = null)
    {
        if (!state.Invincible) //무적인지 체크
        {
            if (remainingHp == null)
                state.hp -= info.Damage;
            else
                state.hp = (float)remainingHp;

            TestUI.Instance.PrintText("player" + OwnerId + "hp : " + state.hp + " / Attacker : " + attackerId);

            charRigidbody.AddForce(new Vector2(info.ImpactX, info.ImpactY));

            lastAttackedPlayerId = attackerId;

            InvokeEventGetHit(CurrentPosition, CurrentVelocity, attackerId, info, state.hp);

            if (IsLocalPlayer && state.hp <= 0)
            {
                PlayerDie();
            }
        }
    }

    public override void PlayerDie()
    {
        if (!isDead)
        {
            isDead = true;
            InvokeEventDead(CurrentPosition, lastAttackedPlayerId);
            gameObject.SetActive(false);
            StopCoroutine("KeepSync");
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

    public override void GetItem(FieldItem item)
    {
        if(item.ItemType == ItemType.Recover)
        {
            if (item.Amount == 0)
                RecoverHp(DEFAULT_HP_RECOVER_AMOUNT);
            else
                RecoverHp(item.Amount);
        }
        else if(item.ItemType == ItemType.Weapon)
        {
            if (item.Amount == 0)
                Inventory.AddItem(item.WeaponId);
            else
                Inventory.AddItem(item.WeaponId, item.Amount);
        }
        //InvokeEventGetItem(item.ItemId);
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
                InvokeEventSync(charRigidbody.position, charRigidbody.velocity, state);
            }
            yield return new WaitForSeconds(NetworkModule.SyncFrequency);
        }
    }

}
