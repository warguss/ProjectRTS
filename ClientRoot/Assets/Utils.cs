using UnityEngine;

namespace RTS
{
    public struct PosInt
    {
        public int x;
        public int y;
    }

    public struct PosFloat
    {
        public float x;
        public float y;
    }

    public struct PlayerInput
    {
        public bool left;
        public bool right;
        public bool jump;
        public bool fire;
        public bool roll;
    }

    public enum PlayerAction
    {
        Stop,
        Left,
        Right,
        Jump,
        Fire,
        Roll
    }

    public class Utils
    {
        // Use this for initialization

    }
}
