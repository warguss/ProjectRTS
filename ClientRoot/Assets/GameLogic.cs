﻿using UnityEngine;
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

    public int myId = -1;

    Dictionary<int, MainCharacter> playerCharacters; 
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
        MainCharacter player = playerCharacters[id];
        if (isMe)
        {
            myId = id;
            CameraScript.SetTarget(playerCharacters[myId].gameObject);

            player.MoveEvent += PlayerEventMove;
            player.StopEvent += PlayerEventStop;
            player.JumpEvent += PlayerEventJump;
            player.ShootEvent += PlayerEventShoot;
            player.GetHitEvent += PlayerEventGetHit;
        }

        TestUI.Instance.PrintText("User Join : " + id + isMe);
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
        int SenderId = packetBody.SenderId;
        switch (packetBody.MsgType)
        {
            case PacketBody.Types.messageType.UserConnection:
                message = "UserConnection Data Received. id=" + packetBody.Connect.ConnectorId;
                TestUI.Instance.PrintText(message);
                ProcessConnectionPacket(packetBody.Connect);
                break;
            case PacketBody.Types.messageType.GameEvent:
                message = "GameEvent Data Received. id=" + packetBody.Event.InvokerId + ", position = " + packetBody.Event.EventPositionX + " , " + packetBody.Event.EventPositionY;
                TestUI.Instance.PrintText(message);
                ProcessEventPacket(packetBody.Event);
                break;
            default:
                message = "UnknownType : " + (int)packetBody.MsgType;
                TestUI.Instance.PrintText(message);
                break;
        }
        
    }

    void ProcessConnectionPacket(UserConnection ConnectionPacket)
    {
        int connectorId = ConnectionPacket.ConnectorId;

        if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.AcceptConnect)
        {
            isConnected = true;
            NetworkModule.instance.myId = connectorId;
            userJoin(connectorId, true);
        }
        else if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.Connect)
        {
            userJoin(connectorId);
        }
        else if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.DisConnect)
        {
            //DeletePlayer(ConnectionPacket.Id);
        }
        else
        {
            TestUI.Instance.PrintText("wrong Packet Received: " + ConnectionPacket.ConType);
        }
    }

    void ProcessEventPacket(GameEvent ConnectionPacket)
    {
        int invokerId = ConnectionPacket.InvokerId;
        int actionProperty = ConnectionPacket.ActionProperty;
        playerCharacters[invokerId].MoveTo(ConnectionPacket.EventPositionX, ConnectionPacket.EventPositionY,
                                          ConnectionPacket.VelocityX, ConnectionPacket.VelocityY);

        switch (ConnectionPacket.Act)
        {
            case GameEvent.Types.action.Move:
                if(actionProperty == 0)
                {
                    SendInputToCharacter(invokerId, PlayerAction.Left);
                }
                else
                {
                    SendInputToCharacter(invokerId, PlayerAction.Right);
                }
                break;

            case GameEvent.Types.action.Stop:
                SendInputToCharacter(invokerId, PlayerAction.Stop);
                break;

            case GameEvent.Types.action.Jump:
                SendInputToCharacter(invokerId, PlayerAction.Jump);
                break;

            case GameEvent.Types.action.Shoot:
                SendInputToCharacter(invokerId, PlayerAction.Fire);
                break;

            case GameEvent.Types.action.GetHit:
                playerCharacters[invokerId].GetHit(10, 10, 50, 0);//////////////////
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
        var currentVelocity = playerCharacters[myId].CurrentVelocity;
        NetworkModule.instance.WriteEventMove(currentPosition, currentVelocity, isLeft);
    }

    void PlayerEventStop()
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        var currentVelocity = playerCharacters[myId].CurrentVelocity;
        NetworkModule.instance.WriteEventStop(currentPosition, currentVelocity);
    }

    void PlayerEventGetHit(int damage, int hitRecovery, int impact, int impactAngle)
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        var currentVelocity = playerCharacters[myId].CurrentVelocity;
        NetworkModule.instance.WriteEventGetHit(currentPosition, currentVelocity, damage, hitRecovery, impact, impactAngle);
    }

    void PlayerEventJump()
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        var currentVelocity = playerCharacters[myId].CurrentVelocity;
        NetworkModule.instance.PlayerEventJump(currentPosition, currentVelocity);
    }

    void PlayerEventShoot(bool isLeft)
    {
        var currentPosition = playerCharacters[myId].CurrentPosition;
        var currentVelocity = playerCharacters[myId].CurrentVelocity;
        NetworkModule.instance.WriteEventMove(currentPosition, currentVelocity,isLeft);
    }
}
   