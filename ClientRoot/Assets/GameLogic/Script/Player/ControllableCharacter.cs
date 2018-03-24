using UnityEngine;
using System.Collections;

public struct HitInfo
{
    public int Damage;
    public int HitRecovery;
    public int Impact;
    public int ImpactAngle;
}

public delegate void CharEventMove(Vector2 position, Vector2 velocity, bool isLeft);
public delegate void CharEventStop(Vector2 position, Vector2 velocity);
public delegate void CharEventJump(Vector2 position, Vector2 velocity);
public delegate void CharEventGetHit(Vector2 position, Vector2 velocity, HitInfo info);
public delegate void CharEventShoot(Vector2 position, Vector2 velocity, bool isLeft);
public delegate void CharEventSpawn(Vector2 position);
public delegate void CharEventDead(int PlayerId);

public interface IControllableCharacter
{
    void SetOwner(int owner);

    void MoveLeft();
    void MoveRight();
    void MoveStop();
    void Jump();
    void Shoot();
    void Spawn(Vector2 position);
    void Dead();

    void SetLocation(Vector2 position);
    void MoveTo(Vector2 position, Vector2 velocity);

    void GetHit(HitInfo info);

    event CharEventMove MoveEvent;
    event CharEventStop StopEvent;
    event CharEventJump JumpEvent;
    event CharEventGetHit GetHitEvent;
    event CharEventShoot ShootEvent;
    event CharEventSpawn SpawnEvent;
    event CharEventDead DeadEvent;

    GameObject GetGameObject();
}
