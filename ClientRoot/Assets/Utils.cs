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
        public static PosInt RbPosToGamePos(float x, float y)
        {
            int posX = (int)(x * 100);
            int posY = (int)(y * 100);
            PosInt posint = new PosInt
            {
                x = posX,
                y = posY
            };
            return posint;
        }

        public static PosInt RbPosToGamePos(Vector2 rbPos)
        {
            return RbPosToGamePos(rbPos.x, rbPos.y);
        }

        public static PosFloat GamePosToRbPos(int x, int y)
        {
            float posX = (float)(x / 100);
            float posY = (float)(y / 100);
            PosFloat posfloat = new PosFloat
            {
                x = posX,
                y = posY
            };
            return posfloat;
        }

        public static int GamePosDistance(PosInt a, PosInt b)//////////////////////////
        {
            Vector2 vecA = new Vector2(a.x, a.y);
            Vector2 vecB = new Vector2(b.x, b.y);
            float vecDistance = Vector2.Distance(vecA, vecB);

            return (int)vecDistance;
        }
    }
}
