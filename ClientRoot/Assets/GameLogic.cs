using UnityEngine;
using System.Collections;
using Google.Protobuf;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;
using RTS;
using Server2N;

public class GameLogic : MonoBehaviour
{
    static public GameLogic Instance;

    public GameObject PlayerPrefab;
    public CameraMove CameraScript;

    Dictionary<int, MainCharacter> playerCharacters;
    int myId = -1; 
    //int testId = 2;

    byte[] msgBuffer = new byte[256];

    bool isConnected = false;

    // Use this for initialization
    void Start()
    {
        Instance = this;
        playerCharacters = new Dictionary<int, MainCharacter>();

        //userJoin(0, true);

        //CameraScript.SetTarget(playerCharacters[myId].gameObject);
    }
	
    // Update is called once per frame
    void Update()
    {
        CheckPacket();
        ProcessInput();
    }

    void FixedUpdate()
    {
    }

    void userJoin(int id, bool isMe = false)
    {
        AddPlayer(id);
        if(isMe)
        {
            MainCharacter player = playerCharacters[id];

            myId = id;
            CameraScript.SetTarget(playerCharacters[myId].gameObject);

            player.MoveEvent += PlayerEventMove;
            player.StopEvent += PlayerEventStop;
            player.JumpEvent += PlayerEventJump;
            player.ShootEvent += PlayerEventShoot;
            player.GetHitEvent += PlayerEventGetHit;
        }
    }

    void userLeave(int id)
    {
    }

    void AddPlayer(int playerId)
    {
        MainCharacter characterScript = Instantiate(PlayerPrefab).GetComponent<MainCharacter>();
        playerCharacters.Add(playerId, characterScript);
        characterScript.playerId = playerId;
    }

    void SendInputToCharacter(int player, PlayerAction action)
    {
        if (playerCharacters.ContainsKey(player))
        {
            playerCharacters[player].SetInput(action);
        }
    }

    void ProcessInput()
    {
        if (myId != -1)
        {
            if (Input.GetKeyDown(KeyCode.LeftArrow))
                SendInputToCharacter(myId, PlayerAction.Left);
            else if (Input.GetKeyDown(KeyCode.RightArrow))
                SendInputToCharacter(myId, PlayerAction.Right);
            else if (Input.GetKeyUp(KeyCode.LeftArrow) || Input.GetKeyUp(KeyCode.RightArrow))
                SendInputToCharacter(myId, PlayerAction.Stop);

            if (Input.GetKeyDown(KeyCode.UpArrow))
                SendInputToCharacter(myId, PlayerAction.Jump);
            if (Input.GetKeyDown(KeyCode.RightShift))
                SendInputToCharacter(myId, PlayerAction.Fire);

        }

        //if (Input.GetKeyDown(KeyCode.A))
        //    SendInputToCharacter(testId, PlayerAction.Left);
        //else if (Input.GetKeyDown(KeyCode.D))
        //    SendInputToCharacter(testId, PlayerAction.Right);
        //else if(Input.GetKeyUp(KeyCode.A) || Input.GetKeyUp(KeyCode.D))
        //    SendInputToCharacter(testId, PlayerAction.Stop);

        //if (Input.GetKeyDown(KeyCode.W))
        //    SendInputToCharacter(testId, PlayerAction.Jump);
        //if (Input.GetKeyDown(KeyCode.Q))
        //    SendInputToCharacter(testId, PlayerAction.Fire);
        
    }

    void ProcessPacketBody(PacketBody packetBody)
    {
        string message = "";
        switch (packetBody.MsgType)
        {
            case PacketBody.Types.messageType.UserConnection:
                message = "UserConnection Data Received. id=" + packetBody.Connect.Id;
                ProcessConnectionPacket(packetBody.Connect);
                break;
            case PacketBody.Types.messageType.GameEvent:
                message = "GameEvent Data Received. position=" + packetBody.Event.EventPositionX + " , " + packetBody.Event.EventPositionY;
                ProcessEventPacket(packetBody.Event);
                break;
            default:
                message = "UnknownType : " + (int)packetBody.MsgType;
                break;
        }
        Debug.Log(message);
        //TestUI.Instance.PrintText(message);
    }

    void ProcessConnectionPacket(UserConnection ConnectionPacket)
    {
        if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.AcceptConnect)
        {
            isConnected = true;
            NetworkModule.instance.myId = ConnectionPacket.Id;
            userJoin(ConnectionPacket.Id, true);
        }
        else if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.Connect)
        {
            userJoin(ConnectionPacket.Id);
        }
        else if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.DisConnect)
        {
            //DeletePlayer(ConnectionPacket.Id);
        }
        else
        {
            Debug.Log("wrong Packet Received: " + ConnectionPacket.ConType);
        }
    }

    void ProcessEventPacket(GameEvent ConnectionPacket)
    {
        int id = ConnectionPacket.Id;
        int actionProperty = ConnectionPacket.ActionProperty;
        playerCharacters[id].MoveTo(ConnectionPacket.EventPositionX, ConnectionPacket.EventPositionY);
        switch (ConnectionPacket.Act)
        {
            case GameEvent.Types.action.Move:
                if(actionProperty == 0)
                {
                    SendInputToCharacter(id, PlayerAction.Left);
                }
                else
                {
                    SendInputToCharacter(id, PlayerAction.Right);
                }
                break;

            case GameEvent.Types.action.Stop:
                SendInputToCharacter(id, PlayerAction.Stop);
                break;

            case GameEvent.Types.action.Jump:
                SendInputToCharacter(id, PlayerAction.Jump);
                break;

            case GameEvent.Types.action.Shoot:
                SendInputToCharacter(id, PlayerAction.Fire);
                break;

            case GameEvent.Types.action.GetHit:
                playerCharacters[id].GetHit(10, 10, 50, 0);//////////////////
                break;
        }
    }

    void CheckPacket()
    {
        PacketBody Packet = NetworkModule.instance.GetReceivedPacketBody();
        if (Packet != null)
        {
            ProcessPacketBody(Packet);
        }
    }

    void PlayerEventMove(bool isLeft)
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        NetworkModule.instance.WriteEventMove(currentPosition, isLeft);
    }

    void PlayerEventStop()
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        NetworkModule.instance.WriteEventStop(currentPosition);
    }

    void PlayerEventGetHit(int damage, int hitRecovery, int impact, int impactAngle)
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        NetworkModule.instance.WriteEventMove(currentPosition, damage, hitRecovery, impact, impactAngle);
    }

    void PlayerEventJump()
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        NetworkModule.instance.PlayerEventJump(currentPosition);
    }

    void PlayerEventShoot(bool isLeft)
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        NetworkModule.instance.WriteEventMove(currentPosition, isLeft);
    }
}
   