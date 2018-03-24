using RTS;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class PlayerController {
    private int mPlayerId = -1;

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
    public string PlayerName { get; set; }

    public IControllableCharacter Character { get; set; }

    public void SetInput(PlayerAction action)
    {
        switch (action)
        {
            case PlayerAction.Stop:
                Character.MoveStop();
                break;

            case PlayerAction.Left:
                Character.MoveLeft();
                break;

            case PlayerAction.Right:
                Character.MoveRight();
                break;

            case PlayerAction.Jump:
                Character.Jump();
                break;

            case PlayerAction.Fire:
                Character.Shoot();
                break;
        }
    }

    public void MoveTo(Vector2 position)
    {
        Character.SetLocation(position);
    }

    public void MoveTo(Vector2 position, Vector2 velocity)
    {
        Character.MoveTo(position, velocity);
    }

    public void GetHit(HitInfo info)
    {
        Character.GetHit(info);
    }

    public void Spawn(Vector2 position)
    {
        Character.Spawn(position);
    }

    public void Dead()
    {
        Character.Dead();
    }

    public void LeaveGame()
    {
       
    }
}
