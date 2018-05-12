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

public interface IControllableCharacter
{
    void SetOwner(int owner);
    void SetName(string name);

    void MoveLeft();
    void MoveRight();
    void MoveStop();
    void Jump();
    void Shoot();
    void ShootWithDamageInfo(DamageInfo info, Vector2 position);
    void ShootWithDamageInfo(DamageInfo info);
    void Spawn(Vector2 position);
    void Dead();

    void InitialSync();

    void SetLocation(Vector2 position);
    void MoveTo(Vector2 position, Vector2 velocity);

    void GetHit(DamageInfo info);

    event CharEventMove MoveEvent;
    event CharEventStop StopEvent;
    event CharEventJump JumpEvent;
    event CharEventGetHit GetHitEvent;
    event CharEventShoot ShootEvent;
    event CharEventSpawn SpawnEvent;
    event CharEventDead DeadEvent;

    GameObject GetGameObject();
}
