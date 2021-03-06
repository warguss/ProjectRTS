﻿using UnityEngine;
using System.Collections;
using System.Collections.Generic;

public delegate void CharEventMove(int invokerId, Vector2 position, Vector2 velocity, bool isLeft);
public delegate void CharEventStop(int invokerId, Vector2 position, Vector2 velocity);
public delegate void CharEventJump(int invokerId, Vector2 position, Vector2 velocity);
public delegate void CharEventGetHit(int invokerId, Vector2 position, Vector2 velocity, int attackerId, HitInfo info, float remainingHp);
public delegate void CharEventShoot(int invokerId, Vector2 position, Vector2 velocity, ShootInfo info, WeaponId weaponId);
public delegate void CharEventChangeWeapon(int invokerId, Vector2 position, Vector2 velocity, WeaponId WeaponId);
public delegate void CharEventSpawn(int invokerId, Vector2 position);
public delegate void CharEventDead(int invokerId, Vector2 position, int attackerId);
//public delegate void CharEventGetItem(int invokerId, string itemId);
public delegate void CharEventSync(int invokerId, Vector2 position, Vector2 velocity, CharacterStateInfo info);
public delegate void CharEventRoll(int invokerId, Vector2 position, Vector2 velocity);

public abstract class ControllableCharacter : MonoBehaviour
{
    public GameObject PlayerInfoDisplay;

    public int OwnerId { get; set; }
    public string OwnerName { get; set; }

    public bool IsLocalPlayer = false;

    public PlayerInventory Inventory;

    protected Rigidbody2D charRigidbody;
    protected Collider2D charCollider;
    protected Collider2D charHitBox;
    protected GameObject charSpriteObject;

    protected PlayerInfoDisplay playerInfoDisplay;
    protected SpriteOverlayScript SpriteOverlay;

    protected CharacterStateInfo state;

    public bool IsLeft { get { return state.IsLeft;  } }

    public event CharEventMove MoveEvent;
    public event CharEventStop StopEvent;
    public event CharEventJump JumpEvent;
    public event CharEventGetHit GetHitEvent;
    public event CharEventShoot ShootEvent;
    public event CharEventChangeWeapon ChangeWeaponEvent;
    public event CharEventSpawn SpawnEvent;
    public event CharEventDead DieEvent;
    //public event CharEventGetItem GetItemEvent;
    public event CharEventSync SyncEvent;
    public event CharEventRoll RollEvent;

    protected void InvokeEventMove(Vector2 position, Vector2 velocity, bool isLeft)
    {
        MoveEvent?.Invoke(OwnerId, position, velocity, isLeft);
    }
    protected void InvokeEventStop(Vector2 position, Vector2 velocity)
    {
        StopEvent?.Invoke(OwnerId, position, velocity);
    }
    protected void InvokeEventJump(Vector2 position, Vector2 velocity)
    {
        JumpEvent?.Invoke(OwnerId, position, velocity);
    }
    protected void InvokeEventGetHit(Vector2 position, Vector2 velocity, int attackerId, HitInfo info, float remainingHp)
    {
        GetHitEvent?.Invoke(OwnerId, position, velocity, attackerId, info, remainingHp);
    }
    protected void InvokeEventShoot(Vector2 position, Vector2 velocity, ShootInfo info, WeaponId weaponId)
    {
        ShootEvent?.Invoke(OwnerId, position, velocity, info, weaponId);
    }
    protected void InvokeEventChangeWeapon(Vector2 position, Vector2 velocity, WeaponId weaponId)
    {
        ChangeWeaponEvent?.Invoke(OwnerId, position, velocity, weaponId);
    }
    protected void InvokeEventSpawn(Vector2 position)
    {
        SpawnEvent?.Invoke(OwnerId, position);
    }
    protected void InvokeEventDead(Vector2 position, int attackerId)
    {
        DieEvent?.Invoke(OwnerId, position, attackerId);
    }
    //protected void InvokeEventGetItem(string itemId)
    //{
    //    GetItemEvent?.Invoke(OwnerId, itemId);
    //}
    protected void InvokeEventSync(Vector2 position, Vector2 velocity, CharacterStateInfo info)
    {
        SyncEvent?.Invoke(OwnerId, position, velocity, info);
    }

    protected void InvokeEventRoll(Vector2 position, Vector2 velocity)
    {
        RollEvent?.Invoke(OwnerId, position, velocity);
    }

    public void Start()
    {
        
    }

    public void SetOwner(int owner)
    {
        OwnerId = owner;
    }

    public void SetName(string name)
    {
        OwnerName = name;
        if (playerInfoDisplay != null)
        {
            playerInfoDisplay.SetName(OwnerName);
        }
    }

    public void RemoveCharacter()
    {
        Destroy(gameObject);
    }

    public void SetVisible(bool isVisible)
    {
        if (!state.IsDead)
            gameObject.SetActive(isVisible);
        Debug.Log("Player" + OwnerName + " SetVisible : " + isVisible);
    }

    public void SetHP(float inHp)
    {
        state.hp = inHp;   
    }

    public void RecoverHp(float inHp)
    {
        state.hp += inHp;
        if (state.hp > state.MaxHP)
            state.hp = state.MaxHP;
    }

    public Vector2 GetCurrentPosition()
    {
        return charRigidbody.position;
    }

    public CharacterStateInfo GetCharacterState()
    {
        return state;
    }

    public void SetCharacterSpecialState(CharacterSpecialState specialState, float time)
    {
        state.SetSpecialState(specialState, time);
    }

    public abstract void MoveLeft();
    public abstract void MoveRight();
    public abstract void MoveStop();
    public abstract void Jump();
    public abstract void Shoot();
    public abstract void ShootWithShootInfo(ShootInfo info, Vector2 position);
    public abstract void ShootWithShootInfo(ShootInfo info);
    public abstract void ChangeWeapon(WeaponId inWeaponId);
    public abstract void ChangeToNextWeapon();
    public abstract void Spawn(Vector2 position);
    public abstract void PlayerDie();
    public abstract void GetItem(FieldItem item);

    public abstract void InitialSync();

    public abstract void SetLocation(Vector2 position);
    public abstract void MoveWithInterpolation(Vector2 position, Vector2 velocity);

    public abstract void GetHit(int attackerId, HitInfo info, float? remainingHp = null);

    public abstract GameObject GetGameObject();
}
