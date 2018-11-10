using UnityEngine;
using UnityEditor;

public enum CharacterState
{
    Neutral,
    Attacking,
    Attacked,
}

public class CharacterStateInfo
{
    public CharacterState CurrentState = CharacterState.Neutral;
    public float DamageMultiply = 1;
    public float ShootSpeedMultiply = 1;
    public float RapidSpeedMultiply = 1;
    public float MoveSpeedMultiply = 1;
    public float JumpMultiply = 1;
    public bool Invincible = false;

    private float invincibleTimeLeft = 0f;

    public void SetInvincible(float time)
    {
        Invincible = true;
        if (time > invincibleTimeLeft)
            invincibleTimeLeft = time;
    }
    public void UpdateStateInfo(float deltaTime)
    {
        if (invincibleTimeLeft > 0)
        {
            invincibleTimeLeft -= deltaTime;
            if (invincibleTimeLeft < 0)
            {
                invincibleTimeLeft = 0;
                Invincible = false;
            }
        }
    }
}