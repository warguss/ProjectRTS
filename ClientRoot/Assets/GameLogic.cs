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

    public InputInterface inputInterface;

    public GameObject PlayerPrefab;
    public GameObject MapDataPrefab;
    public CameraMove CameraScript;
    public bool isOnline = false;

    public int myId = -1;

    Dictionary<int, PlayerController> playerControllers;
    MapData mapData;
    int testId2P = 1;

    byte[] msgBuffer = new byte[256];

    bool TestMode = false;

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
            userJoin(0, "test1p", true);
            userJoin(testId2P, "test2p", false);
            SpawnPlayer(testId2P, new Vector2(2, 2));
        }
    }
	
    // Update is called once per frame
    void Update()
    {
        CheckPacket();
        ProcessInput();
    }

    void FixedUpdate()
    {
        foreach (KeyValuePair<int, PlayerController> entry in playerControllers)
        {
            entry.Value.DoInputFrame();
        }
    }

    void userJoin(int id, string name, bool isMe = false)
    {
        AddPlayer(id, name);
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
            player.Character.DeadEvent += PlayerEventDead;

            SpawnPlayer(id, new Vector2(2, 2));
        }

        TestUI.Instance.PrintText("User Join : " + id + isMe);
    }

    void userLeave(int id)
    {
        PlayerController player = playerControllers[id];
        player.LeaveGame();
        playerControllers.Remove(id);
    }

    void AddPlayer(int playerId, string playerName)
    {
        if (!playerControllers.ContainsKey(playerId))
        {
            MainCharacter characterScript = Instantiate(PlayerPrefab, new Vector3(1,3,0), new Quaternion()).GetComponent<MainCharacter>();
            PlayerController controller = new PlayerController();
            controller.Character = characterScript;
            controller.PlayerId = playerId;
            controller.PlayerName = playerName;

            playerControllers.Add(playerId, controller);
        }            
        else
        {
            TestUI.Instance.PrintText("Trying to add duplicate user");
        }
    }

    void SendInputToCharacter(int player, PlayerAction action, bool active = true)
    {
        if (playerControllers.ContainsKey(player))
        {
            playerControllers[player].SetInput(action, active);
        }
    }

    void ProcessInput()
    {
        if (myId != -1)
        {
            //SendInputToCharacter(myId, PlayerAction.Left, Input.GetKey(KeyCode.LeftArrow));
            //SendInputToCharacter(myId, PlayerAction.Right, Input.GetKey(KeyCode.RightArrow));
            SendInputToCharacter(myId, PlayerAction.Left, InputDirection.Left == inputInterface.GetCurrentDirection());
            SendInputToCharacter(myId, PlayerAction.Right, InputDirection.Right == inputInterface.GetCurrentDirection());

            if (inputInterface.GetJump())
                SendInputToCharacter(myId, PlayerAction.Jump);
            if (inputInterface.GetFire())
                SendInputToCharacter(myId, PlayerAction.Fire);
        }

        if(TestMode)
        {
            SendInputToCharacter(testId2P, PlayerAction.Left, Input.GetKey(KeyCode.A));
            SendInputToCharacter(testId2P, PlayerAction.Right, Input.GetKey(KeyCode.D));

            if (Input.GetKeyDown(KeyCode.W))
                SendInputToCharacter(testId2P, PlayerAction.Jump);
            if (Input.GetKeyDown(KeyCode.LeftShift))
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
        for (int i = 0; i< ConnectionPacket.ConnectorId.Count; i++)
        {
            int connectorId = ConnectionPacket.ConnectorId[i];
            string connectorName = ConnectionPacket.Nickname[i];
            int connectorKillCount = ConnectionPacket.KillInfo[i];
            int ConnectorDeathCount = ConnectionPacket.DeathInfo[i];

            TestUI.Instance.PrintText("Connector Id : " + connectorId
                                     + " / Name = "+ connectorName
                                     + " / Kill = " + connectorKillCount
                                     + " / Death = " + ConnectorDeathCount);

            if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.AcceptConnect)
            {
                isOnline = true;
                NetworkModule.instance.myId = connectorId;
                userJoin(connectorId, connectorName, true);
            }
            else if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.Connect)
            {
                userJoin(connectorId, connectorName);
                if (playerControllers.ContainsKey(myId))
                    playerControllers[myId].Character.InitialSync();
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
            Vector2 position = new Vector2(EventPacket.EventPositionX, EventPacket.EventPositionY);
            Vector2 velocity = new Vector2(EventPacket.VelocityX, EventPacket.VelocityY);

            playerControllers[invokerId].MoveTo(position, velocity);

            switch (EventPacket.ActType)
            {
                case GameEvent.Types.action.Nothing:
                    break;

                case GameEvent.Types.action.EventMove:
                    {
                        var info = EventPacket.MoveEvent;
                        if(info.Type == EventMove.Types.Direction.Left)
                        {
                            SendInputToCharacter(invokerId, PlayerAction.Left);
                        }
                        else if (info.Type == EventMove.Types.Direction.Right)
                        {
                            SendInputToCharacter(invokerId, PlayerAction.Right);
                        }
                        break;
                    }
                case GameEvent.Types.action.EventStop:
                    {
                        var info = EventPacket.StopEvent;
                        SendInputToCharacter(invokerId, PlayerAction.Stop);
                        break;
                    }

                case GameEvent.Types.action.EventJump:
                    {
                        var info = EventPacket.JumpEvent;
                        SendInputToCharacter(invokerId, PlayerAction.Jump);
                        break;
                    }

                case GameEvent.Types.action.EventShoot:
                    {
                        var info = EventPacket.ShootEvent;
                        DamageInfo damageInfo = new DamageInfo
                        {
                            AttackerId = invokerId,
                            Damage = (int)info.Damage,
                            shootAngle = info.Angle,
                            HitRecovery = 10,/////////unused
                            Impact = info.Impact,/////////////
                            ImpactAngle = info.ImpactAngle///////////
                        };
                        SendInputToCharacter(invokerId, PlayerAction.Fire);
                        playerControllers[invokerId].ShootWithDamageInfo(damageInfo, position);
                        break;
                    }

                case GameEvent.Types.action.EventHit:
                    {
                        var info = EventPacket.HitEvent;
                        DamageInfo damageInfo = new DamageInfo
                        {
                            AttackerId = info.Attacker,
                            Damage = (int)info.Damage,
                            HitRecovery = 10,/////////unused
                            Impact = info.Impact,/////////////
                            ImpactAngle = info.ImpactAngle///////////
                        };
                        playerControllers[invokerId].GetHit(damageInfo);
                        break;
                    }

                case GameEvent.Types.action.EventSpawn:
                    {
                        playerControllers[invokerId].Spawn(position);
                        break;
                    }
                case GameEvent.Types.action.EventDeath:
                    {
                        playerControllers[invokerId].Dead();
                        break;
                    }

                case GameEvent.Types.action.EventUserSync:
                    {
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
        if (isOnline)
        {
            NetworkModule.instance.WriteEventMove(position, velocity, isLeft);
        }
    }

    void PlayerEventStop(Vector2 position, Vector2 velocity)
    {
        if (isOnline)
        {
            NetworkModule.instance.WriteEventStop(position, velocity);
        }
    }

    void PlayerEventGetHit(Vector2 position, Vector2 velocity, DamageInfo info)
    {
        if (isOnline)
        {
            NetworkModule.instance.WriteEventGetHit(position, velocity, info);
        }
    }

    void PlayerEventJump(Vector2 position, Vector2 velocity)
    {
        if (isOnline)
        {
            NetworkModule.instance.WriteEventJump(position, velocity);
        }
    }

    void PlayerEventShoot(Vector2 position, Vector2 velocity, DamageInfo info)
    {
        if (isOnline)
        {
            NetworkModule.instance.WriteEventShoot(position, velocity, info);
        }
    }

    void PlayerEventSpawn(Vector2 position)
    {
        if (isOnline)
        {
            NetworkModule.instance.WriteEventSpawn(position);
        }
    }

    void PlayerEventDead(Vector2 position, int AttackerId)
    {
        StartCoroutine(SpawnAfterSeconds(myId, new Vector2(2, 2), 5));
        if (isOnline)
        {
            NetworkModule.instance.WriteEventDead(position, AttackerId);
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
   