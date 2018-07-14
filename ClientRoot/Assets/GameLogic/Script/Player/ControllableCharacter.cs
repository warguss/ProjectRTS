using UnityEngine;
using System.Collections;

public class DamageInfo
{
    public int AttackerId = -1;
    public float shootAngle;//unuse when hit()
    public int Damage;
    public int HitRecovery;// currently unuse
    public int Impact;
    public int ImpactAngle;
}

public delegate void CharEventMove(Vector2 position, Vector2 velocity, bool isLeft);
public delegate void CharEventStop(Vector2 position, Vector2 velocity);
public delegate void CharEventJump(Vector2 position, Vector2 velocity);
public delegate void CharEventGetHit(Vector2 position, Vector2 velocity, DamageInfo info);
public delegate void CharEventShoot(Vector2 position, Vector2 velocity, DamageInfo info);
public delegate void CharEventSpawn(Vector2 position);
public delegate void CharEventDead(Vector2 position, int attackerId);
public delegate void CharEventSync(Vector2 position, Vector2 velocity);

public abstract class ControllableCharacter : MonoBehaviour
{
    public GameObject PlayerInfoDisplay;

    public int OwnerId { get; set; }
    public string OwnerName { get; set; }

    protected Rigidbody2D charRigidbody;
    protected Collider2D charCollider;
    protected GameObject charSpriteObject;

    protected PlayerInfoDisplay playerInfoDisplay;

    protected CharacterStatus status = CharacterStatus.Neutral;
    protected int hitRecovery = 0;

    protected float hp;
    protected int jumpCount = 0;
    protected int lastAttackedPlayerId = -1;

    protected bool isGrounded = true;
    protected bool isLeft = true;
    protected bool isMoving = false;
    protected bool isDead = false;

    public event CharEventMove MoveEvent;
    public event CharEventStop StopEvent;
    public event CharEventJump JumpEvent;
    public event CharEventGetHit GetHitEvent;
    public event CharEventShoot ShootEvent;
    public event CharEventSpawn SpawnEvent;
    public event CharEventDead DieEvent;
    public event CharEventSync SyncEvent;

    protected void InvokeEventMove(Vector2 position, Vector2 velocity, bool isLeft)
    {
        MoveEvent?.Invoke(position, velocity, isLeft);
    }
    protected void InvokeEventStop(Vector2 position, Vector2 velocity)
    {
        StopEvent?.Invoke(position, velocity);
    }
    protected void InvokeEventJump(Vector2 position, Vector2 velocity)
    {
        JumpEvent?.Invoke(position, velocity);
    }
    protected void InvokeEventGetHit(Vector2 position, Vector2 velocity, DamageInfo info)
    {
        GetHitEvent?.Invoke(position, velocity, info);
    }
    protected void InvokeEventShoot(Vector2 position, Vector2 velocity, DamageInfo info)
    {
        ShootEvent?.Invoke(position, velocity, info);
    }
    protected void InvokeEventSpawn(Vector2 position)
    {
        SpawnEvent?.Invoke(position);
    }
    protected void InvokeEventDead(Vector2 position, int attackerId)
    {
        DieEvent?.Invoke(position, attackerId);
    }
    protected void InvokeEventSync(Vector2 position, Vector2 velocity)
    {
        SyncEvent?.Invoke(position, velocity);
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
        if (!isDead)
            gameObject.SetActive(isVisible);
    }

    public void SetHP(float inHp)
    {
        hp = inHp;   
    }

    public abstract void MoveLeft();
    public abstract void MoveRight();
    public abstract void MoveStop();
    public abstract void Jump();
    public abstract void Shoot();
    public abstract void ShootWithDamageInfo(DamageInfo info, Vector2 position);
    public abstract void ShootWithDamageInfo(DamageInfo info);
    public abstract void Spawn(Vector2 position);
    public abstract void PlayerDie();

    public abstract void InitialSync();

    public abstract void SetLocation(Vector2 position);
    public abstract void MoveWithInterpolation(Vector2 position, Vector2 velocity);

    public abstract void GetHit(DamageInfo info);

    public abstract GameObject GetGameObject();
}
