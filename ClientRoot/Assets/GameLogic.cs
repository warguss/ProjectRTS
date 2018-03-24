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
    public GameObject MapDataPrefab;
    public CameraMove CameraScript;

    public int myId = -1;

    Dictionary<int, PlayerController> playerControllers;
    MapData mapData;
    int testId2P = 1;

    byte[] msgBuffer = new byte[256];

    bool isConnected = false;

    bool TestMode = true;

    // Use this for initialization
    void Start()
    {
        Instance = this;
        playerControllers = new Dictionary<int, PlayerController>();
        mapData = Instantiate(MapDataPrefab).GetComponent<MapData>();

        mapData.LoadMap();
        mapData.DrawMap();

        if (TestMode)
        {
            userJoin(0, true);
            userJoin(testId2P, false);
        }
    }
	
    // Update is called once per frame
    void Update()
    {
        CheckPacket();
    }

    void FixedUpdate()
    {
        ProcessInput();
    }

    void userJoin(int id, bool isMe = false)
    {
        AddPlayer(id);
        PlayerController player = playerControllers[id];
        if (isMe)
        {
            myId = id;
            CameraScript.SetTarget(playerControllers[myId].Character.GetGameObject());

            player.Character.MoveEvent += PlayerEventMove;
            player.Character.StopEvent += PlayerEventStop;
            player.Character.JumpEvent += PlayerEventJump;
            player.Character.ShootEvent += PlayerEventShoot;
            player.Character.GetHitEvent += PlayerEventGetHit;
            player.Character.SpawnEvent += PlayerEventSpawn;
        }
        player.Character.DeadEvent += PlayerEventDead;

        TestUI.Instance.PrintText("User Join : " + id + isMe);
    }

    void userLeave(int id)
    {
        PlayerController player = playerControllers[id];
        player.LeaveGame();
        playerControllers.Remove(id);
    }

    void AddPlayer(int playerId)
    {
        if (!playerControllers.ContainsKey(playerId))
        {
            MainCharacter characterScript = Instantiate(PlayerPrefab, new Vector3(1,3,0), new Quaternion()).GetComponent<MainCharacter>();
            PlayerController controller = new PlayerController();
            controller.Character = characterScript;
            controller.PlayerId = playerId;

            playerControllers.Add(playerId, controller);

            SpawnPlayer(playerId, new Vector2(2, 2));
        }            
        else
        {
            TestUI.Instance.PrintText("Trying to add duplicate user");
        }
    }

    void SendInputToCharacter(int player, PlayerAction action)
    {
        if (playerControllers.ContainsKey(player))
        {
            playerControllers[player].SetInput(action);
        }
    }

    void ProcessInput()
    {
        if (myId != -1)
        {
            if (Input.GetKeyDown(KeyCode.LeftArrow))
                SendInputToCharacter(myId, PlayerAction.Left);
            if (Input.GetKeyDown(KeyCode.RightArrow))
                SendInputToCharacter(myId, PlayerAction.Right);
            if (Input.GetKeyUp(KeyCode.LeftArrow) || Input.GetKeyUp(KeyCode.RightArrow))
            {
                if (Input.GetKey(KeyCode.LeftArrow))
                    SendInputToCharacter(myId, PlayerAction.Left);
                else if (Input.GetKey(KeyCode.RightArrow))
                    SendInputToCharacter(myId, PlayerAction.Right);
                else
                    SendInputToCharacter(myId, PlayerAction.Stop);
            }

            if (Input.GetKeyDown(KeyCode.UpArrow))
                SendInputToCharacter(myId, PlayerAction.Jump);
            if (Input.GetKeyDown(KeyCode.RightShift))
                SendInputToCharacter(myId, PlayerAction.Fire);
        }

        if(TestMode)
        {
            if (Input.GetKeyDown(KeyCode.A))
                SendInputToCharacter(testId2P, PlayerAction.Left);
            else if (Input.GetKeyDown(KeyCode.D))
                SendInputToCharacter(testId2P, PlayerAction.Right);
            else if (Input.GetKeyUp(KeyCode.A) || Input.GetKeyUp(KeyCode.D))
                SendInputToCharacter(testId2P, PlayerAction.Stop);

            if (Input.GetKeyDown(KeyCode.W))
                SendInputToCharacter(testId2P, PlayerAction.Jump);
            if (Input.GetKeyDown(KeyCode.Q))
                SendInputToCharacter(testId2P, PlayerAction.Fire);
        }

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
                if (myId != packetBody.Event.InvokerId[0])
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
        TestUI.Instance.PrintText("Connection Type : " + ConnectionPacket.ConType);
        foreach (int connectorId in ConnectionPacket.ConnectorId)
        {
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
                userLeave(connectorId);
            }
            else
            {
                TestUI.Instance.PrintText("wrong Packet Received: " + ConnectionPacket.ConType);
            }
        }
    }

    void ProcessEventPacket(GameEvent EventPacket)
    {
        foreach (int invokerId in EventPacket.InvokerId)
        {
            int actionProperty = EventPacket.ActionProperty;
            Vector2 position = new Vector2(EventPacket.EventPositionX, EventPacket.EventPositionY);
            Vector2 velocity = new Vector2(EventPacket.VelocityX, EventPacket.VelocityY);

            playerControllers[invokerId].MoveTo(position, velocity);

            switch (EventPacket.Act)
            {
                case GameEvent.Types.action.Nothing:
                    break;

                case GameEvent.Types.action.Move:
                    if (actionProperty == 0)
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
                    {
                        HitInfo info = new HitInfo
                        {
                            Damage = 10,
                            HitRecovery = 10,
                            Impact = 50,
                            ImpactAngle = 0
                        };
                        playerControllers[invokerId].GetHit(info);//////////////////
                        break;
                    }
            }
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

    void PlayerEventMove(Vector2 position, Vector2 velocity, bool isLeft)
    {
        if (isConnected)
        {
            NetworkModule.instance.WriteEventMove(position, velocity, isLeft);
        }
    }

    void PlayerEventStop(Vector2 position, Vector2 velocity)
    {
        if (isConnected)
        {
            NetworkModule.instance.WriteEventStop(position, velocity);
        }
    }

    void PlayerEventGetHit(Vector2 position, Vector2 velocity, HitInfo info)
    {
        if (isConnected)
        {
            NetworkModule.instance.WriteEventGetHit(position, velocity, info);
        }
    }

    void PlayerEventJump(Vector2 position, Vector2 velocity)
    {
        if (isConnected)
        {
            NetworkModule.instance.WriteEventJump(position, velocity);
        }
    }

    void PlayerEventShoot(Vector2 position, Vector2 velocity, bool isLeft)
    {
        if (isConnected)
        {
            NetworkModule.instance.WriteEventShoot(position, velocity, isLeft);
        }
    }

    void PlayerEventSpawn(Vector2 position)
    {
        if (isConnected)
        {
            //NetworkModule.instance.WriteEvenSpawn(position);
        }
    }

    void PlayerEventDead(int playerId)
    {
        StartCoroutine(SpawnAfterSeconds(playerId, new Vector2(2, 2), 5));
        if (isConnected)
        {
            //NetworkModule.instance.WriteEventShoot();
        }
    }

    void SpawnPlayer(int playerId, Vector2 position)
    {
        playerControllers[playerId].Spawn(position);
    }

    IEnumerator SpawnAfterSeconds(int playerId, Vector2 position, int seconds)
    {
        yield return new WaitForSeconds(seconds);
        SpawnPlayer(playerId, position);
    }
}
   