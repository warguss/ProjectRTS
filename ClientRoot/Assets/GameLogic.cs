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
    public const int WEAPON_SLOT_COUNT = 3;
    public const float SPAWN_INVINCIBLE_TIME = 3f;
    public const float MIN_INTERPOLATION_DISTANCE = 2f;

    public const float ITEM_INTERVAL_MAX = 30f;
    public const float ITEM_INTERVAL_MIN = 10f;

    static public GameLogic Instance
    {
        get
        {
            GameObject gameLogicObject = GameObject.Find("/GameLogic");
            if (gameLogicObject)
            {
                return gameLogicObject.GetComponent<GameLogic>();
            }
            else
            {
                GameObject tempInstance = new GameObject("GameLogic");
                GameLogic createdGameLogic = tempInstance.AddComponent<GameLogic>();

                return createdGameLogic;
            }
        }
    }

    public InputInterface inputInterface;

    public GameObject PlayerPrefab;
    public GameObject MapDataPrefab;
    public GameObject ItemPrefab;
    public CameraMove CameraScript;

    Dictionary<int, PlayerController> playerControllers;
    MapData mapData;

    float currentItemInterval;
    int createItemCount = 0;

    Dictionary<string, FieldItem> FieldItems;

    private int spectatorTarget = -1;

    int testId1P = 0;
    int testId2P = 1;

    byte[] msgBuffer = new byte[256];

    // Use this for initialization
    void Start()
    {
        
    }

    public void Initialize()
    {
        playerControllers = new Dictionary<int, PlayerController>();
        FieldItems = new Dictionary<string, FieldItem>();
        mapData = Instantiate(MapDataPrefab).GetComponent<MapData>();
    }

    public void StartGame()
    {
        mapData.LoadMap();
        mapData.DrawMap();

        Debug.Log("isTestMode : "+GameStatus.isTestMode);
        if (GameStatus.isTestMode)
        {
            GameStatus.myId = testId1P;
            userJoin(testId1P, "test1p", true);
            userJoin(testId2P, "test2p", false, true);
            SpawnPlayer(testId2P, new Vector2(2, 2));
        }
        else
        {
            if (!NetworkModule.instance.RequestAuthorization())
            {
                return;
            }

            if (!NetworkModule.instance.isConnected)
            {
                NetworkModule.instance.Connect(GameStatus.entryName);
            }
        }
    }
	
    // Update is called once per frame
    void Update()
    {
        ProcessInput();
        if(GameStatus.isOnline)
        {
            CheckPacket();
            if (!NetworkModule.instance.isConnected)
            {
                GameStatus.isOnline = false;
                CleanUpGame();
                TestUI.Instance.PrintText("Disconnected from server.");
                StopAllCoroutines();
            }
        }
    }

    void FixedUpdate()
    {
        foreach (KeyValuePair<int, PlayerController> entry in playerControllers)
        {
            entry.Value.DoInputFrame();
        }
    }

    void userJoin(int id, string name, bool isMe = false, bool isLocalPlayer = false, bool isNewPlayer = true)
    {
        AddPlayer(id, name, isMe || isLocalPlayer);
        PlayerController player = playerControllers[id];
        if (isMe)
        {
            CameraScript.SetTarget(playerControllers[GameStatus.myId].Character.GetGameObject());
            WeaponUI.Instance.SetInventory(player.GetInventory());

            StartCoroutine(ItemSpawnTimer());
        }
        if(isMe || isLocalPlayer)
        { 
            player.Character.MoveEvent += PlayerEventMove;
            player.Character.StopEvent += PlayerEventStop;
            player.Character.JumpEvent += PlayerEventJump;
            player.Character.ShootEvent += PlayerEventShoot;
            player.Character.ChangeWeaponEvent += PlayerEventChangeWeapon;
            player.Character.GetHitEvent += PlayerEventGetHit;
            player.Character.SpawnEvent += PlayerEventSpawn;
            player.Character.DieEvent += PlayerEventDie;
            //player.Character.GetItemEvent += PlayerEventGetItem;
            player.Character.SyncEvent += PlayerEventSync;
        }

        SpawnPlayer(id, new Vector2(2, 2), isNewPlayer);

        TestUI.Instance.PrintText("User Join : " + id + isMe);
    }

    void userLeave(int id)
    {
        PlayerController player = playerControllers[id];
        player.LeaveGame();
        playerControllers.Remove(id);
    }

    void AddPlayer(int playerId, string playerName, bool isLocalPlayer)
    {
        if (!playerControllers.ContainsKey(playerId))
        {
            MainCharacter characterScript = Instantiate(PlayerPrefab, new Vector3(1,3,0), new Quaternion()).GetComponent<MainCharacter>();
            characterScript.IsLocalPlayer = isLocalPlayer;

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
        if (GameStatus.myId != -1)
        {
            //SendInputToCharacter(myId, PlayerAction.Left, Input.GetKey(KeyCode.LeftArrow));
            //SendInputToCharacter(myId, PlayerAction.Right, Input.GetKey(KeyCode.RightArrow));
            SendInputToCharacter(GameStatus.myId, PlayerAction.Left, InputDirection.Left == inputInterface.GetCurrentDirection());
            SendInputToCharacter(GameStatus.myId, PlayerAction.Right, InputDirection.Right == inputInterface.GetCurrentDirection());

            if (inputInterface.GetJump())
                SendInputToCharacter(GameStatus.myId, PlayerAction.Jump);
            if (inputInterface.GetFire())
                SendInputToCharacter(GameStatus.myId, PlayerAction.Fire);

            if (inputInterface.GetRoll())
                SendInputToCharacter(GameStatus.myId, PlayerAction.Roll);

            if (inputInterface.GetNextWeapon())
            {
                playerControllers[GameStatus.myId].ChangeToNextWeapon();
                //playerControllers[myId].ChangeWeapon(keysList[nextIndex]);
            }
        }

        if(GameStatus.isTestMode)
        {
            SendInputToCharacter(testId2P, PlayerAction.Left, Input.GetKey(KeyCode.A));
            SendInputToCharacter(testId2P, PlayerAction.Right, Input.GetKey(KeyCode.D));

            if (Input.GetKeyDown(KeyCode.W))
                SendInputToCharacter(testId2P, PlayerAction.Jump);
            if (Input.GetKeyDown(KeyCode.LeftShift))
                SendInputToCharacter(testId2P, PlayerAction.Fire);
            if(Input.GetKeyDown(KeyCode.Q))
            {
                playerControllers[testId2P].ChangeToNextWeapon();
            }
        }

    }

    public void CleanUpGame()
    {
        GameStatus.isOnline = false;

        foreach (var entry in playerControllers)
        {
            entry.Value.LeaveGame();
        }
        foreach (var entry in FieldItems)
        {
            Destroy(entry.Value.gameObject);
        }

        playerControllers.Clear();
        FieldItems.Clear();
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
                message = "GameEvent Data Received. id=" + packetBody.Event.InvokerId + ",(" + packetBody.Event.EventPositionX + ", " + packetBody.Event.EventPositionY + ")";
                TestUI.Instance.PrintText(message);
                ProcessEventPacket(packetBody.Event);
                break;
            case PacketBody.Types.messageType.GlobalNotice:
                message = "GlobalNotice data Received.";
                TestUI.Instance.PrintText(message);
                ProcessNoticePacket(packetBody.Notice);
                break;
            default:
                message = "UnknownType packet received: " + (int)packetBody.MsgType;
                TestUI.Instance.PrintText(message);
                break;
        }
        
    }

    void ProcessConnectionPacket(UserConnection ConnectionPacket)
    {
        TestUI.Instance.PrintText("Connection Type : " + ConnectionPacket.ConType);
        if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.Connect) // Connect타입 패킷에서 아이템을 받는다? 처음에만 받고 이후 유저가 접속할 때는 없음. 개선예정?
        {
            TestUI.Instance.PrintText("Created Item Count : " + ConnectionPacket.Item.Count);
            for (int j = 0; j < ConnectionPacket.Item.Count; j++)
            {
                string itemId = ConnectionPacket.Item[j].ItemId;
                float itemPositionX = ConnectionPacket.Item[j].ItemPositionX;
                float itemPositionY = ConnectionPacket.Item[j].ItemPositionY;
                int itemType = (int)ConnectionPacket.Item[j].ItemType;
                int weaponId = (int)ConnectionPacket.Item[j].WeaponId;
                int amount = ConnectionPacket.Item[j].Amount;

                CreateItem(new Vector2(itemPositionX, itemPositionY), itemId, (ItemType)itemType, (WeaponId)weaponId, amount);
            }
        }

        for (int i = 0; i< ConnectionPacket.ConnectorId.Count; i++)
        {
            int connectorId = ConnectionPacket.ConnectorId[i];

            if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.AcceptConnect)
            {
                string connectorName = ConnectionPacket.Nickname[i];
                //int connectorKillCount = ConnectionPacket.KillInfo[i];
                //int ConnectorDeathCount = ConnectionPacket.DeathInfo[i];

                //TestUI.Instance.PrintText("Connector Id : " + connectorId
                //                         + " / Name = " + connectorName
                //                         + " / Kill = " + connectorKillCount
                //                         + " / Death = " + ConnectorDeathCount);

                GameStatus.isOnline = true;
                GameStatus.myId = connectorId;
                userJoin(connectorId, connectorName, true);
            }
            else if (ConnectionPacket.ConType == UserConnection.Types.ConnectionType.Connect)
            {
                string connectorName = ConnectionPacket.Nickname[i];

                int connectorKillCount = ConnectionPacket.KillInfo[i];
                int ConnectorDeathCount = ConnectionPacket.DeathInfo[i];

                TestUI.Instance.PrintText("Connector Id : " + connectorId
                                         + " / Name = " + connectorName
                                         + " / Kill = " + connectorKillCount
                                         + " / Death = " + ConnectorDeathCount);
               
                userJoin(connectorId, connectorName, false, false, false);
                if (playerControllers.ContainsKey(GameStatus.myId))
                    playerControllers[GameStatus.myId].Character.InitialSync();
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
        TestUI.Instance.PrintText("ProcessEventPacket: " + EventPacket.EvtType);

        Vector2 position = new Vector2(EventPacket.EventPositionX, EventPacket.EventPositionY);
        Vector2 velocity = new Vector2(EventPacket.VelocityX, EventPacket.VelocityY);

        switch(EventPacket.EvtType)
        {
            case GameEvent.Types.eventType.UserEvent:
                {
                    foreach (int invokerId in EventPacket.InvokerId)
                    {
                        PlayerController currentPlayer = playerControllers[invokerId];
                        bool isInterested = EventPacket.IsInterested;//TO DO
                        if (invokerId == GameStatus.myId)
                        {
                            continue;
                        }
                        currentPlayer.IsInterested = !isInterested; // 서버에선 값이 반대?
                        if (Vector2.Distance(currentPlayer.GetCurrentPosition(), position) > MIN_INTERPOLATION_DISTANCE)
                        {
                            currentPlayer.MoveWithInterpolation(position, velocity);
                        }

                        var userEventPacket = EventPacket.UserEvent;
                        ProcessUserEvent(userEventPacket, invokerId, position);
                    }
                    break;
                }
            case GameEvent.Types.eventType.SystemEvent:
                {
                    foreach (int invokerId in EventPacket.InvokerId)
                    {
                        var systemEventPacket = EventPacket.SystemEvent;
                        ProcessSystemEvent(systemEventPacket, invokerId, position);
                    }
                    break;
                }
        }
    }

    void ProcessUserEvent(UserEvent userEventPacket, int invokerId, Vector2 position)
    {
            switch (userEventPacket.ActType)
            {
                case UserEvent.Types.action.Nothing:
                    break;

                case UserEvent.Types.action.EventMove:
                    {

                        var info = userEventPacket.MoveEvent;
                        if (info.Type == EventMove.Types.Direction.Left)
                        {
                            SendInputToCharacter(invokerId, PlayerAction.Left);
                        }
                        else if (info.Type == EventMove.Types.Direction.Right)
                        {
                            SendInputToCharacter(invokerId, PlayerAction.Right);
                        }
                        break;
                    }
                case UserEvent.Types.action.EventStop:
                    {

                        var info = userEventPacket.StopEvent;
                        SendInputToCharacter(invokerId, PlayerAction.Stop);
                        break;
                    }

                case UserEvent.Types.action.EventJump:
                    {

                        var info = userEventPacket.JumpEvent;
                        SendInputToCharacter(invokerId, PlayerAction.Jump);
                        break;
                    }

                case UserEvent.Types.action.EventShoot:
                    {

                        var info = userEventPacket.ShootEvent;
                        int weaponId = info.ShootType;
                        ShootInfo shootInfo = new ShootInfo
                        {
                            ShootType = (WeaponId)weaponId,
                            Damage = info.Damage,
                            ShootAngle = info.ShootAngle,
                            ImpactScale = info.ImpactScale,/////////////
                            BulletRange = info.BulletRange,
                            BulletSpeed = info.BulletSpeed,
                        };
                        //SendInputToCharacter(invokerId, PlayerAction.Fire);
                        playerControllers[invokerId].ChangeWeapon((WeaponId)weaponId);
                        playerControllers[invokerId].ShootWithShootInfo(shootInfo, position);
                        break;
                    }

                case UserEvent.Types.action.EventChangeWeapon:
                    {

                        var info = userEventPacket.ChWeaponEvent;
                        int weaponId = info.WeaponId;
                        playerControllers[invokerId].ChangeWeapon((WeaponId)weaponId);
                        break;
                    }

                case UserEvent.Types.action.EventHit:
                    {

                        var info = userEventPacket.HitEvent;
                    HitInfo shootInfo = new HitInfo
                    {
                        HitType = (WeaponId)info.HitType,
                        Damage = info.Damage,
                        ImpactX = info.ImpactX,/////////////
                        ImpactY = info.ImpactY
                        };
                        float currentHp = info.CurrentHP;
                        playerControllers[invokerId].GetHit(info.Attacker, shootInfo, currentHp);
                        break;
                    }

                case UserEvent.Types.action.EventSpawn:
                    {
                        SpawnPlayer(invokerId, position);
                        break;
                    }
                case UserEvent.Types.action.EventDeath:
                    {
                        playerControllers[invokerId].Dead();
                        break;
                    }

                case UserEvent.Types.action.EventUserSync:
                    {
                        var info = userEventPacket.SyncEvent;
                        float currentHp = info.CurrentHP;
                        int weaponId = info.WeaponId;

                        playerControllers[invokerId].SetHP(currentHp);
                        playerControllers[invokerId].ChangeWeapon((WeaponId)weaponId);

                        break;
                    }
            }
    }

    void ProcessSystemEvent(SystemEvent systemEventPacket, int invokerId, Vector2 position)
    {
        switch (systemEventPacket.ActType)
        {
            case SystemEvent.Types.action.EventItemSpawn:
                {
                    var info = systemEventPacket.ItemSpawnEvent;
                    var item = info.Item;
                    string itemId = item.ItemId;

                    TestUI.Instance.PrintText("EventItemSpawn - (" + itemId + "), " + item.ItemType + ", " + item.WeaponId);
                    CreateItem(position, itemId, (ItemType)item.ItemType, (WeaponId)item.WeaponId);
                    break;
                }

            case SystemEvent.Types.action.EventItemGet:
                {
                    var info = systemEventPacket.ItemGetEvent;
                    var item = info.Item;
                    string itemId = item.ItemId;

                    TestUI.Instance.PrintText("EventItemGet - (" + itemId + "), " + invokerId);
                    ConsumeItem(invokerId, itemId);
                    break;
                }

            case SystemEvent.Types.action.RequestUserInfo:
                {
                    var info = systemEventPacket.RequestUserInfo;
                    int targetId = info.TargetID;
                    float xPos = info.EventPositionX;
                    float yPos = info.EventPositionY;

                    TestUI.Instance.PrintText("Receive RequestUserInfo - (" + targetId + "), " + xPos + ", " + yPos);
                    StartCoroutine(KeepSpectatorSync(new Vector2(xPos, yPos)));

                    break;
                }
        }
    }

    void ProcessNoticePacket(GlobalNotice noticePacket)
    {
        switch(noticePacket.NotiType)
        {
            case GlobalNotice.Types.NoticeInfo.KillInfo:
                int performerId = noticePacket.Performer;
                var victimId = noticePacket.Victim;

                string performerName = playerControllers[performerId].PlayerName;
                string victimName = playerControllers[victimId[0]].PlayerName;
                KillLogUi.Instance.GenerateKillLog(performerName, victimName);
                TestUI.Instance.PrintText("perfomer : " + noticePacket.Performer + "(" + performerName + ")" + 
                    " / victim : " + noticePacket.Victim + "(" + victimName + ")");
                break;

            case GlobalNotice.Types.NoticeInfo.Notice:
                TestUI.Instance.PrintText("message : " + noticePacket.Notice);
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

    void PlayerEventMove(int invokerId, Vector2 position, Vector2 velocity, bool isLeft)
    {
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventMove(invokerId, position, velocity, isLeft);
        }
    }

    void PlayerEventStop(int invokerId, Vector2 position, Vector2 velocity)
    {
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventStop(invokerId, position, velocity);
        }
    }

    void PlayerEventGetHit(int invokerId, Vector2 position, Vector2 velocity, int attackerId, HitInfo info, float remainingHp)
    {
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventGetHit(invokerId, position, velocity, attackerId, info, remainingHp);
        }
    }

    void PlayerEventJump(int invokerId, Vector2 position, Vector2 velocity)
    {
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventJump(invokerId, position, velocity);
        }
    }

    void PlayerEventShoot(int invokerId, Vector2 position, Vector2 velocity, ShootInfo info, WeaponId weaponId)
    {
        PlayerController player = playerControllers[GameStatus.myId];
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventShoot(invokerId, position, velocity, info, weaponId);
        }
    }

    void PlayerEventChangeWeapon(int invokerId, Vector2 position, Vector2 velocity, WeaponId weaponId)
    {
        PlayerController player = playerControllers[GameStatus.myId];
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventChangeWeapon(invokerId, position, velocity, weaponId);
        }
    }

    void PlayerEventSpawn(int invokerId, Vector2 position)
    {
        CameraScript.SetTarget(playerControllers[GameStatus.myId].Character.GetGameObject()); 
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventSpawn(invokerId, position);
        }
    }

    void PlayerEventDie(int invokerId, Vector2 position, int AttackerId)
    {
        StartCoroutine(SpawnAfterSeconds(invokerId, new Vector2(Random.Range(1f, 10f), Random.Range(1f, 10f)), 5));
        //StartCoroutine(KeepSpectatorSync(position));
        spectatorTarget = AttackerId;
        CameraScript.SetTarget(playerControllers[AttackerId].Character.GetGameObject()); //TODO : 일정 딜레이 후 시점 바뀌는 걸로 바꾸기
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventDead(invokerId, position, AttackerId);
            NetworkModule.instance.WriteRequestUserPosition(invokerId, AttackerId);
        }
    }

    void PlayerEventGetItem(int invokerId, string itemId)
    {
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventGetItem(invokerId, itemId);
        }
    }

    void PlayerEventSync(int invokerId, Vector2 position, Vector2 velocity, CharacterStateInfo info)
    {
        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventSync(invokerId, position, velocity, info);
        }
    }

    void SpawnPlayer(int playerId, Vector2 position, bool setInvincible = true)
    {
        playerControllers[playerId].Spawn(position);
        spectatorTarget = -1;
        if (setInvincible)
            playerControllers[playerId].SetInvincible(SPAWN_INVINCIBLE_TIME);
    }

    IEnumerator SpawnAfterSeconds(int playerId, Vector2 position, int seconds)
    {
        yield return new WaitForSeconds(seconds);
        SpawnPlayer(playerId, position);
    }

    public void CreateItem(Vector2 position, string itemId, ItemType type, WeaponId weapon, int amount = 0)
    {
        TestUI.Instance.PrintText("CreateItem" + "(" + itemId + "), " + type + ", " + weapon);
        GameObject created = Instantiate(ItemPrefab, new Vector3(position.x, position.y, 0), new Quaternion());
        FieldItem item = created.GetComponent<FieldItem>();
        item.Initialize(itemId, type, weapon, amount);

        FieldItems.Add(itemId, item);
    }

    public void ConsumeItem(int targetPlayer, string itemId)
    {
        if (FieldItems.ContainsKey(itemId))
        {
            var targetItem = FieldItems[itemId];

            if (targetPlayer == GameStatus.myId)
            {
                playerControllers[GameStatus.myId].GetItem(targetItem);
            }
            else
            {
                //효과만 표시?
            }

            Destroy(targetItem.gameObject);
            FieldItems.Remove(itemId);
        }
        else
        {

        }
    }

    public void CreateItemRandomely(Vector2 centerPosition, float range)
    {
        float xMin = centerPosition.x - range;
        float xMax = centerPosition.x + range;
        float yMin = centerPosition.y - range;
        float yMax = centerPosition.y + range;

        if (xMin < 0)
            xMin = 0;
        if (yMin < 0)
            yMin = 0;

        Vector2 itemPosition = new Vector2(Random.Range(xMin, xMax), Random.Range(yMin, yMax));
        string itemId = GenerateItemId();
        TestUI.Instance.PrintText("CreateItemRandomely (" + itemId + "), " + itemPosition);
        ItemType itemType = (ItemType)Random.Range(0, 2);
        WeaponId weaponId = WeaponId.None;
        int itemAmount = 0;
        switch (itemType)
        {
            case ItemType.Recover:
                {
                    break;
                }
            case ItemType.Weapon:
                {
                    int weaponIdInt = Random.Range(1, 3);
                    weaponId = (WeaponId)weaponIdInt;
                    break;
                }
        }

        if (GameStatus.isOnline)
        {
            NetworkModule.instance.WriteEventSpawnItem(GameStatus.myId, itemPosition, itemId, itemType, weaponId, itemAmount);
        }
        else
        {
            CreateItem(itemPosition, itemId, itemType, weaponId, itemAmount);
        }
    }

    public string GenerateItemId()
    {
        createItemCount++;
        string itemId = System.Guid.NewGuid().ToString();

        return itemId;
    }

    IEnumerator ItemSpawnTimer()
    {
        while (true)
        {
            currentItemInterval = Random.Range(ITEM_INTERVAL_MIN, ITEM_INTERVAL_MAX);
            while (currentItemInterval > 0)
            {
                yield return null;
                currentItemInterval -= Time.deltaTime;
            }
            CreateItemRandomely(playerControllers[GameStatus.myId].GetCurrentPosition(), 10);
        }
    }

    IEnumerator KeepSpectatorSync(Vector2 startPosition)
    {
        TestUI.Instance.PrintText("KeepSpectatorSync Start, target : " + spectatorTarget);
        NetworkModule.instance.WriteEventSync(GameStatus.myId, startPosition, new Vector2(), new CharacterStateInfo());
        yield return new WaitForSeconds(NetworkModule.SyncFrequency);
        while(spectatorTarget != -1)
        {
            NetworkModule.instance.WriteEventSync(GameStatus.myId, playerControllers[spectatorTarget].GetCurrentPosition(), new Vector2(), new CharacterStateInfo());
            yield return new WaitForSeconds(NetworkModule.SyncFrequency);
        }
        TestUI.Instance.PrintText("KeepSpectatorSync End");
    }
}
   