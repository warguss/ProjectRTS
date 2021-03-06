﻿using UnityEngine;
using System.Collections;
using Server2N;
using Google.Protobuf;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;
using System;
using System.Threading;

public class NetworkModule : MonoBehaviour
{
    public const string SERVER_IP = "ec2-13-125-18-197.ap-northeast-2.compute.amazonaws.com";
    public const int SERVER_PORT = 10001;
    public const float SyncFrequency = 0.2f;
    public const float MaxInterpolationTime = 0.3f;
    public const float InterpolationLongestDistance = 5;
    public const float SERVER_TIMEOUT = 15;
    public const int ZERO_OFFSET = 0;

    public static NetworkModule instance;
    public bool isConnected = false;
    public bool isAuthSuccess = false;

    TcpClient client;
    NetworkStream ns;
    Thread readThread;
    Thread writeThread;

    Packet readPacket;
    Packet writePacket;

    Queue<PacketBody> RecevedPacketBodyQueue;
    Queue<PacketBody> SendPacketBodyQueue;

    object ReadQueueLock;
    object WriteQueueLock;

    //int testCount = 0;

    private void Awake()
    {
        if (instance != null)
        {
            Destroy(gameObject);
        }
        instance = this;
    }

    // Use this for initialization
    void Start()
    {
        readPacket = new Packet();
        writePacket = new Packet();
        RecevedPacketBodyQueue = new Queue<PacketBody>();
        SendPacketBodyQueue = new Queue<PacketBody>();
        ReadQueueLock = new object();
        WriteQueueLock = new object();
    }
	
    // Update is called once per frame
    void Update()
    {
        
    }
    void NetworkReadThreadRoutine()
    {
        while (true)
        {
            if (client != null && ns != null)
            {
                if (ns.DataAvailable)
                {
                    readPacket.AllocateRawData(0);
                    ns.Read(readPacket.RawData, ZERO_OFFSET, Packet.HEADER_LENGTH);
                    if (readPacket.Decode_Header())
                    {
                        byte[] packetBodyByteArray;
                        packetBodyByteArray = new byte[(int)readPacket.BodyLength];
                        ns.Read(packetBodyByteArray, ZERO_OFFSET, (int)readPacket.BodyLength);

                        Debug.Log("Decode_Header : " + readPacket.BodyLength);
                        var parsed = PacketBody.Parser.ParseFrom(packetBodyByteArray);
                        lock (ReadQueueLock)
                        {
                            RecevedPacketBodyQueue.Enqueue(parsed);
                        }
                    }
                    else
                    {
                        //헤더 디코딩 에러
                    }
                    readPacket.Clean();
                }
                else
                {
                    
                }

            }
        }
    }

    void NetworkWriteThreadRoutine()
    {
        while (true)
        {
            if (client != null && ns != null)
            {
                PacketBody packet = null;
                lock (SendPacketBodyQueue)
                {
                    if (SendPacketBodyQueue.Count > 0)
                    {
                        packet = SendPacketBodyQueue.Dequeue();
                    }
                }
                if (packet != null)
                    SendPacket(packet);
            }
        }
    }

    public void Connect(string name = "")
    {
        try
        {
            readThread = new Thread(NetworkReadThreadRoutine);
            readThread.Start();
            writeThread = new Thread(NetworkWriteThreadRoutine);
            writeThread.Start();

            TestUI.Instance.PrintText("Connected");
            isConnected = true;
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText(e.Message);
        }

        if (client != null && ns != null)
        {
            WriteTryConnection(name);
        }
    }

    public void Disconnect()
    {
        TestUI.Instance.PrintText("Trying to disconnect");
        try
        {
            if (client != null)
            {
                ConnectionCleanUp();
            }
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText(e.Message);
            CheckIsConnected();
        }
    }

    void ConnectionCleanUp()
    {
        ns.Close();
        client.Close();
        readThread.Abort();
        writeThread.Abort();

        ns = null;
        client = null;
        readThread = null;
        writeThread = null;

        isConnected = false;
    }

    private String _ReadStrBuffer()
    {
        String recvBuffer = "";
        try
        {
            int bufferSize = 1024;
            byte[] readBuffer = new byte[bufferSize];
            ns.Read(readBuffer, ZERO_OFFSET, bufferSize);
            recvBuffer = System.Text.Encoding.Default.GetString(readBuffer);
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText("Read Buffer Failed : " + e.Message + "\n" + e.StackTrace);
            return recvBuffer;
        }

        return recvBuffer;
    }


    private bool _SendStrBuffer(string buffer)
    {
        if ( buffer.Length <= 0 )
        {
            return false;
        }

        try
        {
            byte[] writeBuffer = System.Text.Encoding.UTF8.GetBytes(buffer);
            ns.Write(writeBuffer, ZERO_OFFSET, writeBuffer.Length);
            Debug.Log("BufferSended type : " + buffer + ", bufferLength : " + writeBuffer.Length);
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText("Send Buffer Failed : " + e.Message + "\n" + e.StackTrace);
            return false;
        }

        return true;
    }

    public void SendPacket(PacketBody packet)
    {
        try
        {
            byte[] writeBuffer;
            writeBuffer = packet.ToByteArray();
            writePacket.AllocateRawData(writeBuffer.Length);
            writeBuffer.CopyTo(writePacket.RawData, Packet.HEADER_LENGTH);
            writePacket.Encode_Header();

            //string hex = BitConverter.ToString(writeBuffer);
            //Debug.Log("packetBody : " + hex);

            ns.Write(writePacket.RawData, ZERO_OFFSET, writePacket.RawData.Length);
            //TestUI.Instance.PrintText("PacketSended type : " + packet.MsgType + ", BodyLength : " + writePacket.BodyLength);
            Debug.Log("PacketSended type : " + packet.MsgType + ", BodyLength : " + writePacket.BodyLength);

            writePacket.Clean();

        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText("Send Failed : " + e.Message + "\n" + e.StackTrace);
            CheckIsConnected();
        }
    }

    PacketBody CreateConnectionPacket(UserConnection.Types.ConnectionType type, int id, string nickName  = "")
    {
        UserConnection uesrConnection = new UserConnection
        {
            ConType = type,
        };
        if(string.IsNullOrEmpty(nickName))
        {
            uesrConnection.Nickname.Add("User" + Time.fixedTime.ToString());
        }
        else
        {
            uesrConnection.Nickname.Add(nickName);
        }

        uesrConnection.ConnectorId.Add(id);

        PacketBody packetBody = new PacketBody
        {
            MsgType = PacketBody.Types.messageType.UserConnection,
            Connect = uesrConnection,
        };

        return packetBody;
    }

    PacketBody CreateGameEventPacket(int id, Vector2 position, Vector2 velocity)
    {
        GameEvent gameEvent = new GameEvent
        {
            EventPositionX = position.x,
            EventPositionY = position.y,
            VelocityX = velocity.x,
            VelocityY = velocity.y,
        };
        gameEvent.InvokerId.Add(id);

        PacketBody packetBody = new PacketBody()
        {
            MsgType = PacketBody.Types.messageType.GameEvent,
            Event = gameEvent,
        };

        return packetBody;
    }

    PacketBody CreateUserEventPacket(UserEvent.Types.action type, int id, Vector2 position, Vector2 velocity)
    {

        PacketBody packetBody = CreateGameEventPacket(id, position, velocity);
        UserEvent userEvent = new UserEvent
        {
            ActType = type
        };
        packetBody.Event.EvtType = GameEvent.Types.eventType.UserEvent;
        packetBody.Event.UserEvent = userEvent;

        return packetBody;
    }

    PacketBody CreateSystemEventPacket(SystemEvent.Types.action type, int id, Vector2 position, Vector2 velocity)
    {
        PacketBody packetBody = CreateGameEventPacket(id, position, velocity);
        SystemEvent systemEvent = new SystemEvent
        {
            ActType = type
        };
        packetBody.Event.EvtType = GameEvent.Types.eventType.SystemEvent;
        packetBody.Event.SystemEvent = systemEvent;

        return packetBody;
    }

    public PacketBody GetReceivedPacketBody()
    {
        lock (ReadQueueLock)
        {
            if (RecevedPacketBodyQueue.Count > 0)
                return RecevedPacketBodyQueue.Dequeue();
            else
                return null;
        }
    }

    void EnqueueSendPacket(PacketBody packet)
    {
        lock(WriteQueueLock)
        {
            SendPacketBodyQueue.Enqueue(packet);
        }
    }

    void CheckIsConnected()
    {
        try
        {
            if (client != null && client.Client != null && client.Client.Connected)
            {
                /* pear to the documentation on Poll:
                 * When passing SelectMode.SelectRead as a parameter to the Poll method it will return 
                 * -either- true if Socket.Listen(Int32) has been called and a connection is pending;
                 * -or- true if data is available for reading; 
                 * -or- true if the connection has been closed, reset, or terminated; 
                 * otherwise, returns false
                 */

                // Detect if client disconnected
                if (client.Client.Poll(0, SelectMode.SelectRead))
                {
                    byte[] buff = new byte[1];
                    if (client.Client.Receive(buff, SocketFlags.Peek) == 0)
                    {
                        // Client disconnected
                        isConnected =  false;
                    }
                    else
                    {
                        isConnected =  true;
                    }
                }

                isConnected = true;
            }
            else
            {
                isConnected = false;
            }
        }
        catch
        {
            isConnected = false;
        }

        if(!isConnected)
        {
            ConnectionCleanUp();
        }
    }

    public void WriteTryConnection(string name = "")
    {
        var packet = CreateConnectionPacket(UserConnection.Types.ConnectionType.TryConnect, 0, name);

        EnqueueSendPacket(packet);
    }

    //////////////////////////////////////////UserEvent

    public void WriteEventSync(int InvokerId, Vector2 position, Vector2 velocity, CharacterStateInfo info)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventUserSync, InvokerId, position, velocity);
        packet.Event.UserEvent.SyncEvent = new EventUserSync
        {
            CurrentHP = info.hp,
            WeaponId = (int)info.CurrentWeapon
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventSpawn(int InvokerId, Vector2 position)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventSpawn, InvokerId, position, new Vector2(0, 0));

        EnqueueSendPacket(packet);
    }

    public void WriteEventDead(int InvokerId, Vector2 position, int AttackerId)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventDeath, InvokerId, position, new Vector2(0, 0));
        packet.Event.UserEvent.DeathEvent = new EventDeath
        {
            TriggerId = AttackerId
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventMove(int InvokerId, Vector2 position, Vector2 velocity, bool isLeft)
    {
        EventMove.Types.Direction direction = EventMove.Types.Direction.Left ;
        if (!isLeft)
            direction = EventMove.Types.Direction.Right;

        var packet = CreateUserEventPacket(UserEvent.Types.action.EventMove, InvokerId, position, velocity);
        packet.Event.UserEvent.MoveEvent = new EventMove
        {
            Type = direction
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventStop(int InvokerId, Vector2 position, Vector2 velocity)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventStop, InvokerId, position, velocity);
        packet.Event.UserEvent.StopEvent = new EventStop
        {

        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventGetHit(int InvokerId, Vector2 position, Vector2 velocity, int attackerId, HitInfo info, float remainingHp)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventHit, InvokerId, position, velocity);
        packet.Event.UserEvent.HitEvent = new EventHit
        {
            Attacker = attackerId,
            HitType = (int)info.HitType,
            ImpactX = info.ImpactX,
            ImpactY = info.ImpactY,
            Damage = info.Damage,
            CurrentHP = remainingHp
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventJump(int InvokerId, Vector2 position, Vector2 velocity)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventJump, InvokerId, position, velocity);
        packet.Event.UserEvent.JumpEvent = new EventJump
        {

        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventShoot(int InvokerId, Vector2 position, Vector2 velocity, ShootInfo info, WeaponId weaponId)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventShoot, InvokerId, position, velocity);
        packet.Event.UserEvent.ShootEvent = new EventShoot
        {
            BulletSpeed = info.BulletSpeed,
            BulletRange = info.BulletRange,
            ImpactScale = info.ImpactScale,
            ShootAngle = info.ShootAngle,
            Damage = info.Damage,
            ShootType = (int)weaponId 
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventChangeWeapon(int InvokerId, Vector2 position, Vector2 velocity, WeaponId weaponId)
    {
        var packet = CreateUserEventPacket(UserEvent.Types.action.EventChangeWeapon, InvokerId, position, velocity);
        packet.Event.UserEvent.ChWeaponEvent = new EventChangeWeapon
        {
            WeaponId = (int)weaponId
        };

        EnqueueSendPacket(packet);
    }

    //////////////////////////////////////////SystemEvent

    public void WriteEventSpawnItem(int InvokerId, Vector2 position, string itemId, ItemType type, WeaponId weapon, int amount)
    {
        var packet = CreateSystemEventPacket(SystemEvent.Types.action.EventItemSpawn, InvokerId, position, new Vector2(0, 0));
        InfoItem infoItem = new InfoItem
        {
            Amount = amount,
            ItemId = itemId,
            ItemType = (InfoItem.Types.ItemType)type,
            WeaponId = (InfoItem.Types.WeaponId)weapon
        };
        packet.Event.SystemEvent.ItemSpawnEvent = new EventItemSpawn
        {
            Item = infoItem
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventGetItem(int InvokerId, string itemId)
    {
        var packet = CreateSystemEventPacket(SystemEvent.Types.action.EventItemGet, InvokerId, new Vector2(0, 0), new Vector2(0, 0));
        InfoItem infoItem = new InfoItem //현재는 Get할 때는 itemId만 사용
        {
            ItemId = itemId
        };
        packet.Event.SystemEvent.ItemGetEvent = new EventItemGet
        {
            Item = infoItem
        };

        TestUI.Instance.PrintText("WriteEventGetItem(" + itemId + "), " + InvokerId);
        EnqueueSendPacket(packet);
    }

    public void WriteRequestUserPosition(int InvokerId, int targetId)
    {
        var packet = CreateSystemEventPacket(SystemEvent.Types.action.RequestUserInfo, InvokerId, new Vector2(0, 0), new Vector2(0, 0));
        RequestUserInfo requestUserInfo = new RequestUserInfo //현재는 Get할 때는 itemId만 사용
        {
            TargetID = targetId
        };
        packet.Event.SystemEvent.RequestUserInfo = requestUserInfo;

        TestUI.Instance.PrintText("WriteRequestUserPosition(" + targetId + "), " + InvokerId);
        EnqueueSendPacket(packet);
    }

    /*********************************************
     * Connect 관련
     * Stream SET하는 부분 공통으로 처리
     *********************************************/
    public bool Initializer(string ip, int port)
    {
        TestUI.Instance.PrintText("Trying to connect " + ip + ":" + port);
        if (client == null)
        {
            try
            {
                client = new TcpClient(ip, port);
                ns = client.GetStream();
            }
            catch (Exception e)
            {
                TestUI.Instance.PrintText(e.Message);
                return false;
            }
        }

        return true;
    }

    /*********************************************
     *  Agent 관련 로직
     *********************************************/
    public bool RequestAuthorization()
    {
        if (isAuthSuccess)
        {
            return true;
        }

        do
        {
            ServiceAuth auth = new ServiceAuth();
            string authKey = auth._getServiceAuthKey();
            if (!_SendStrBuffer(authKey))
            {
                TestUI.Instance.PrintText("Send Str Buffer Error ");
                break;
            }

            /**********************************************
             * Response에 대해 처리
             **********************************************/
            string recvBuffer = _ReadStrBuffer();
            if (recvBuffer.Length <= 0 )
            {
                TestUI.Instance.PrintText("Read Str Buffer Error ");
                break;
            }

            //TestUI.Instance.PrintText("recvBuffer (" + recvBuffer + ")");
            recvBuffer = recvBuffer.ToLower();
            if (!recvBuffer.Contains("success"))
            {
                TestUI.Instance.PrintText("recvBuffer Not Exist");
                break;
            }
            isAuthSuccess = true;
        }
        while (false);

        return isAuthSuccess;
    }
}
