using UnityEngine;
using System.Collections;
using RTS;
using System;
using System.Collections.Generic;

public class MainCharacter : ControllableCharacter
{
    const float DEFAULT_HP_RECOVER_AMOUNT = 30;
    const float DEFAULT_HP = 100;

    const float ROLLING_INVINCIBLE_TIME = 0.3f;

    public float MaxMoveSpeed;
    public float MoveForce;
    public float JumpForce;
    public int MaxJumpCount = 2;
    public int MaxRollCount = 1;

    public float RollForce = 7f;

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
        state.IsDead = false;
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
        state = new CharacterStateInfo();

        state.IsDead = true;
        charRigidbody = GetComponent<Rigidbody2D>();
        charCollider = GetComponent<Collider2D>();
        charHitBox = transform.Find("HitBox").GetComponent<Collider2D>();
        charSpriteObject = transform.Find("Sprite").gameObject;
        SpriteOverlay = transform.Find("SpriteOverlay").GetComponent<SpriteOverlayScript>();

        Inventory = new PlayerInventory(this);

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
        
        if(state.GetSpecialState(CharacterSpecialState.Invincible))
        {
            SpriteOverlay.SetInvincible(true);
        }
        else
        {
            SpriteOverlay.SetInvincible(false);
        }

        if(state.GetSpecialState(CharacterSpecialState.RollingInvincible))
        {
            charHitBox.enabled = false;
            SpriteOverlay.SetInvincible(true);
        }
        else
        {
            charHitBox.enabled = true;
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

        if (state.CurrentState == CharacterState.Neutral && !state.IsMoving && !state.IsRolling)
            Brake();
    }

        public override void MoveLeft()
    {
        if (!state.IsDead && !state.IsRolling)
        {
            if (!state.IsMoving || !state.IsLeft)
                InvokeEventMove(CurrentPosition, CurrentVelocity, true);

            state.IsMoving = true;
            state.IsLeft = true;

            //rb2d.velocity = new Vector2(-MaxMoveSpeed, rb2d.velocity.y);
            if (-1 * charRigidbody.velocity.x < MaxMoveSpeed)
                charRigidbody.AddForce(Vector2.right * -1 * MoveForce);

            if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
                charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);

        }
    }

    public override void MoveRight()
    {
        if (!state.IsDead && !state.IsRolling)
        {
            state.DisableSpecialState(CharacterSpecialState.RollingInvincible);

            if (!state.IsMoving || state.IsLeft)
            {
                InvokeEventMove(CurrentPosition, CurrentVelocity, false);
            }

            state.IsMoving = true;
            state.IsLeft = false;

            //rb2d.velocity = new Vector2(MaxMoveSpeed, rb2d.velocity.y);
            if (1 * charRigidbody.velocity.x < MaxMoveSpeed)
                charRigidbody.AddForce(Vector2.right * 1 * MoveForce);

            if (Mathf.Abs(charRigidbody.velocity.x) > MaxMoveSpeed)
                charRigidbody.velocity = new Vector2(Mathf.Sign(charRigidbody.velocity.x) * MaxMoveSpeed, charRigidbody.velocity.y);
        }
    }

    public override void MoveStop()
    {
        if(state.CurrentState == CharacterState.Neutral && !state.IsRolling)
        {
            if (state.IsMoving)
                InvokeEventStop(CurrentPosition, CurrentVelocity);

            state.IsMoving = false;

            state.CurrentState = CharacterState.Neutral;
        }
    }

    void Brake()
    {
        charRigidbody.velocity = new Vector2(charRigidbody.velocity.x * (float)0.8, charRigidbody.velocity.y);
    }

    public override void InitialSync()
    {
        if (!state.IsDead)
        {
            InvokeEventSpawn(CurrentPosition);
            if (state.IsMoving)
            {
                if (state.IsLeft)
                    InvokeEventMove(CurrentPosition, CurrentVelocity, true);
                else
                    InvokeEventMove(CurrentPosition, CurrentVelocity, false);
            }
        }
    }

    public override void Jump()
    {
        if (state.jumpCount < MaxJumpCount)
        {
            state.jumpCount++;

            //rb2d.AddForce(Vector2.up*JumpForce);
            charRigidbody.velocity = new Vector2(charRigidbody.velocity.x, JumpForce);

            InvokeEventJump(CurrentPosition, CurrentVelocity);
        }
    }

    public override void Shoot()
    {
        if (!state.IsDead)
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
        if (!state.IsDead)
        {
            state.DisableSpecialState(CharacterSpecialState.RollingInvincible);

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
        int wallLayer = LayerMask.NameToLayer("Wall");
        int playerLayer = LayerMask.NameToLayer("Player");
        int mask = (1 << wallLayer) | (1 << playerLayer);

        var checkGrounded = Physics2D.Raycast(transform.position, -Vector2.up, distToGround + (float)0.1, mask);

        if (checkGrounded)
        {
            if (state.IsGrounded)
            {

            }
            else
            {
                Land();
            }
        }
        else
        {
            if (state.IsGrounded)
            {
                state.IsGrounded = false;
            }
            else
            {

            }
        }
    }

    void Land()
    {
        Debug.Log("Landed");
        state.IsGrounded = true;
        endRoll();
        state.jumpCount = 0;
    }

    public override void GetHit(int attackerId, HitInfo info, float? remainingHp = null)
    {
        if (!state.GetSpecialState(CharacterSpecialState.Invincible) && !state.GetSpecialState(CharacterSpecialState.RollingInvincible)) //무적인지 체크
        {
            if (remainingHp == null)
                state.hp -= info.Damage;
            else
                state.hp = (float)remainingHp;

            TestUI.Instance.PrintText("player" + OwnerId + "hp : " + state.hp + " / Attacker : " + attackerId);

            charRigidbody.AddForce(new Vector2(info.ImpactX, info.ImpactY));

            state.lastAttackedPlayerId = attackerId;

            InvokeEventGetHit(CurrentPosition, CurrentVelocity, attackerId, info, state.hp);

            if (IsLocalPlayer && state.hp <= 0)
            {
                PlayerDie();
            }
        }
    }

    public override void PlayerDie()
    {
        if (!state.IsDead)
        {
            state.IsDead = true;
            InvokeEventDead(CurrentPosition, state.lastAttackedPlayerId);
            gameObject.SetActive(false);
            StopCoroutine("KeepSync");
        }
    }

    public override void SetLocation(Vector2 position)
    {
        if (!state.IsDead)
        {
            gameObject.transform.position = position;
            //charRigidbody.MovePosition(position);
        }
    }

    public override void MoveWithInterpolation(Vector2 position, Vector2 velocity)
    {
        if (!state.IsDead)
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
            if (!state.IsDead)
            {
                InvokeEventSync(charRigidbody.position, charRigidbody.velocity, state);
            }
            yield return new WaitForSeconds(NetworkModule.SyncFrequency);
        }
    }

    public void DoRoll()
    {
        if (!state.IsRolling && !state.IsDead && state.rollCount < MaxRollCount)
        {
            state.IsRolling = true;
            state.rollCount++;

            state.SetSpecialState(CharacterSpecialState.RollingInvincible, ROLLING_INVINCIBLE_TIME);

            Vector2 directionVector;
            if (state.IsLeft)
                directionVector = Vector2.left;
            else
                directionVector = Vector2.right;

            //charRigidbody.velocity = new Vector2(0, 0);
            //charRigidbody.AddForce((Vector2.up + directionVector) * RollForce);

            charRigidbody.velocity = (Vector2.up * 0.6f + directionVector) * RollForce;

            InvokeEventRoll(CurrentPosition, CurrentVelocity);
        }
    }

    public void endRoll()
    {
        state.IsRolling = false;
        state.rollCount = 0;

        state.DisableSpecialState(CharacterSpecialState.RollingInvincible);
    }

}
