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
    }

    public enum PlayerAction
    {
        Stop,
        Left,
        Right,
        Jump,
        Fire
    }

    public class Utils
    {
        // Use this for initialization

    }
}
