using RTS;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class CurrentInput
{
    public bool Left = false;
    public bool Right = false;
    public bool Jump = false;
    public bool Fire = false;
}

public class PlayerController {
    private int mPlayerId = -1;
    private string mPlayerName = "";
    private CurrentInput currentInput = new CurrentInput();

    public int PlayerId
    {
        get
        {
            return mPlayerId;
        }
        set
        {
            mPlayerId = value;
            Character.SetOwner(mPlayerId);
        }
    }
    public string PlayerName
    {
        get
        {
            return mPlayerName;
        }
        set
        {
            mPlayerName = value;
            Character.SetName(mPlayerName);
        }
    }

    public ControllableCharacter Character { get; set; } //TODO : private로 만들기

    public void DoInputFrame()
    {
        ControllByInput();
    }

    public void SetInput(PlayerAction action, bool active = true)
    {
        switch (action)
        {
            case PlayerAction.Left:
                currentInput.Left = active;
                break;

            case PlayerAction.Right:
                currentInput.Right = active;
                break;

            case PlayerAction.Jump:
                currentInput.Jump = active;
                break;

            case PlayerAction.Fire:
                currentInput.Fire = active;
                break;

            case PlayerAction.Stop:
                currentInput.Left = false;
                currentInput.Right = false;
                break;
        }
    }

    public void ControllByInput()
    {
        if (currentInput.Left)
        {
            Character.MoveLeft();
        }
        else if (currentInput.Right)
        {
            Character.MoveRight();
        }
        else
        {
            Character.MoveStop();
        }
        
        if (currentInput.Jump)
        {
            Character.Jump();
            currentInput.Jump = false;
        }

        if (currentInput.Fire)
        {
            Character.Shoot();
            currentInput.Fire = false;
        }
    }

    public void MoveWithInterpolation(Vector2 position, Vector2 velocity)
    {
        Character.MoveWithInterpolation(position, velocity);
    }

    public void ShootWithDamageInfo(DamageInfo info, Vector2 position)
    {
        Character.ShootWithDamageInfo(info, position);
    }

    public void GetHit(DamageInfo info)
    {
        Character.GetHit(info);
    }

    public void InitialSync()
    {
        Character.InitialSync();
    }

    public void Spawn(Vector2 position)
    {
        Character.Spawn(position);
    }

    public void Dead()
    {
        Character.PlayerDie();
    }

    public void LeaveGame()
    {
        Character.RemoveCharacter();
    }
}
