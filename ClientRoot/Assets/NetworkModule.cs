using UnityEngine;
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
    public const string SERVER_IP = "210.89.191.141";
    public const int SERVER_PORT = 10001;
    public const float SyncFrequency = 0.2f;
    public const float MaxInterpolationTime = 0.3f;
    public const float InterpolationLongestDistance = 5;
    public const float SERVER_TIMEOUT = 15;

    public static NetworkModule instance;
    public bool isConnected = false;

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

    private int MyId
    {
        set
        {
            GameLogic.Instance.myId = value;
        }
        get
        {
            return GameLogic.Instance.myId;
        }

    }

    //int testCount = 0;

    // Use this for initialization
    void Start()
    {
        instance = this;

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
                    ns.Read(readPacket.RawData, 0, Packet.HEADER_LENGTH);
                    if (readPacket.Decode_Header())
                    {
                        byte[] packetBodyByteArray;
                        packetBodyByteArray = new byte[(int)readPacket.BodyLength];
                        ns.Read(packetBodyByteArray, 0, (int)readPacket.BodyLength);

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
   

        public void Connect(string ip, int port, string name = "")
    {
        TestUI.Instance.PrintText("Trying to connect " + ip + ":" + port);

        if (client == null)
        {
            try
            {
                client = new TcpClient(ip, port);
                ns = client.GetStream();
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

            ns.Write(writePacket.RawData, 0, writePacket.RawData.Length);
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

    PacketBody CreateCommonEventPacket(GameEvent.Types.action type, int id, Vector2 position, Vector2 velocity)
    {

        GameEvent gameEvent = new GameEvent
        {
            ActType = type,
            EventPositionX = position.x,
            EventPositionY = position.y,
            VelocityX = velocity.x,
            VelocityY = velocity.y,
            //ActionProperty = actionProperty,
        };
        gameEvent.InvokerId.Add(id);

        PacketBody packetBody = new PacketBody()
        {
            MsgType = PacketBody.Types.messageType.GameEvent,
            Event = gameEvent,
        };

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

    public void WriteEventSync(int InvokerId, Vector2 position, Vector2 velocity)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventUserSync, InvokerId, position, velocity);

        EnqueueSendPacket(packet);
    }

    public void WriteEventSpawn(int InvokerId, Vector2 position)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventSpawn, InvokerId, position, new Vector2(0, 0));

        EnqueueSendPacket(packet);
    }

    public void WriteEventDead(int InvokerId, Vector2 position, int AttackerId)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventDeath, InvokerId, position, new Vector2(0, 0));
        packet.Event.DeathEvent = new EventDeath
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

        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventMove, InvokerId, position, velocity);
        packet.Event.MoveEvent = new EventMove
        {
            Type = direction
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventStop(int InvokerId, Vector2 position, Vector2 velocity)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventStop, InvokerId, position, velocity);
        packet.Event.StopEvent = new EventStop
        {

        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventGetHit(int InvokerId, Vector2 position, Vector2 velocity, int attackerId, DamageInfo info)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventHit, InvokerId, position, velocity);
        packet.Event.HitEvent = new EventHit
        {
            Attacker = attackerId,
            ImpactAngle = info.ImpactAngle,
            Impact = info.Impact,
            Damage = info.Damage
        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventJump(int InvokerId, Vector2 position, Vector2 velocity)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventJump, InvokerId, position, velocity);
        packet.Event.JumpEvent = new EventJump
        {

        };

        EnqueueSendPacket(packet);
    }

    public void WriteEventShoot(int InvokerId, Vector2 position, Vector2 velocity, DamageInfo info)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventShoot, InvokerId, position, velocity);
        packet.Event.ShootEvent = new EventShoot
        {
            Impact = info.Impact,
            ImpactAngle = info.ImpactAngle,
            Angle = info.shootAngle,
            Damage = info.Damage
        };

        EnqueueSendPacket(packet);
    }
}
