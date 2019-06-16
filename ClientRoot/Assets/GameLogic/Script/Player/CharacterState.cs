using UnityEngine;
using UnityEditor;

public enum CharacterState
{
    Neutral,
    Attacking,
    Attacked,
}

public enum CharacterSpecialState : int
{
    Invincible = 0,
    RollingInvincible = 1
}

public class CharacterStateInfo
{
    public const int SPECIAL_STATES_NUMBER = 2;

    public CharacterState CurrentState = CharacterState.Neutral;

    public float hp;
    public float MaxHP;

    public WeaponId CurrentWeapon = WeaponId.None;

    public float DamageMultiply = 1;
    public float ShootSpeedMultiply = 1;
    public float RapidSpeedMultiply = 1;
    public float MoveSpeedMultiply = 1;
    public float JumpMultiply = 1;
    public bool[] SpecialStates = new bool[SPECIAL_STATES_NUMBER];

    public int jumpCount = 0;
    public int rollCount = 0;
    public int hitRecovery = 0;
    public int lastAttackedPlayerId = -1;


    public bool IsGrounded { get; set; }
    public bool IsLeft { get; set; }
    public bool IsMoving { get; set; }
    public bool IsDead { get; set; }
    public bool IsRolling { get; set; }

    private float[] specialStatesLeftTime = new float[SPECIAL_STATES_NUMBER];

    public void SetSpecialState(CharacterSpecialState specialState, float time)
    {
        int specialStateNo = (int)specialState;

        SpecialStates[specialStateNo] = true;
        if (time > specialStatesLeftTime[specialStateNo])
        {
            specialStatesLeftTime[specialStateNo] = time;
        }
    }

    public bool GetSpecialState(CharacterSpecialState specialState)
    {
        return SpecialStates[(int)specialState];
    }

    public void DisableSpecialState(CharacterSpecialState specialState)
    {
        int specialStateNo = (int)specialState;

        SpecialStates[specialStateNo] = false;
        specialStatesLeftTime[specialStateNo] = 0f;
    }

    public void UpdateStateInfo(float deltaTime)
    {
        for (int i = 0; i < SPECIAL_STATES_NUMBER; i++)
        {
            if (specialStatesLeftTime[i] > 0f)
            {
                specialStatesLeftTime[i] -= deltaTime;
                if (specialStatesLeftTime[i] < 0)
                {
                    specialStatesLeftTime[i] = 0;
                    SpecialStates[i] = false;
                }
            }
        }
    }
}