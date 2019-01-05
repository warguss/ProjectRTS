using UnityEngine;
using UnityEditor;

public enum CharacterState
{
    Neutral,
    Attacking,
    Attacked,
}

public enum CharacterSpecialState
{
    Invincible
}

public class CharacterStateInfo
{
    public CharacterState CurrentState = CharacterState.Neutral;

    public float hp;
    public float MaxHP;

    public WeaponId CurrentWeapon = WeaponId.None;

    public float DamageMultiply = 1;
    public float ShootSpeedMultiply = 1;
    public float RapidSpeedMultiply = 1;
    public float MoveSpeedMultiply = 1;
    public float JumpMultiply = 1;
    public bool Invincible = false;

    private float invincibleTimeLeft = 0f;

    public void SetSpecialState(CharacterSpecialState specialState, float time)
    {
        switch(specialState)
        {
            case CharacterSpecialState.Invincible:
                Invincible = true;
                if (time > invincibleTimeLeft)
                    invincibleTimeLeft = time;
                break;
        }
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