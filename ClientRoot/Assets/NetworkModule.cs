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
    public static NetworkModule instance;

    TcpClient client;
    NetworkStream ns;
    Thread readThread;

    Packet readPacket;
    Packet writePacket;

    Queue<PacketBody> RecevedPacketBodyQueue;
    object QueueLock;

    public int myId = -1;

    //int testCount = 0;

    // Use this for initialization
    void Start()
    {
        instance = this;

        readPacket = new Packet();
        writePacket = new Packet();
        RecevedPacketBodyQueue = new Queue<PacketBody>();
        QueueLock = new object();
    }
	
    // Update is called once per frame
    void Update()
    {
        //if (Input.GetKeyDown(KeyCode.Space))
        //{
        //    TestSend();
        //}
    }
    void ReadThreadRoutine()
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
                        lock (QueueLock)
                        {
                            RecevedPacketBodyQueue.Enqueue(parsed);
                        }
                    }
                    else
                    {

                    }

                    readPacket.Clean();
                }
                else
                {

                }
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
                readThread = new Thread(ReadThreadRoutine);
                readThread.Start();
                TestUI.Instance.PrintText("Connected");
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
                ns.Close();
                client.Close();
                readThread.Abort();

                ns = null;
                client = null;
                readThread = null;

                TestUI.Instance.PrintText("Disconnected");
            }
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText(e.Message);
        }
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
        lock (QueueLock)
        {
            if (RecevedPacketBodyQueue.Count > 0)
                return RecevedPacketBodyQueue.Dequeue();
            else
                return null;
        }
    }

    public void WriteTryConnection(string name = "")
    {
        var packet = CreateConnectionPacket(UserConnection.Types.ConnectionType.TryConnect, 0, name);

        SendPacket(packet);
    }

    public void WriteEventUserSync(Vector2 position, Vector2 velocity/*, CurrentAction action = CurrentAction.Idle*/)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventUserSync, myId, position, velocity);
        //packet.Event.ActionProperty

        SendPacket(packet);
    }

    public void WriteEventSpawn(Vector2 position)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventSpawn, myId, position, new Vector2(0, 0));

        SendPacket(packet);
    }

    public void WriteEventDead(Vector2 position, int AttackerId)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventDeath, myId, position, new Vector2(0, 0));
        packet.Event.DeathEvent = new EventDeath
        {
            TriggerId = AttackerId
        };

        SendPacket(packet);
    }

    public void WriteEventMove(Vector2 position, Vector2 velocity, bool isLeft)
    {
        EventMove.Types.Direction direction = EventMove.Types.Direction.Left ;
        if (!isLeft)
            direction = EventMove.Types.Direction.Right;

        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventMove, myId, position, velocity);
        packet.Event.MoveEvent = new EventMove
        {
            Type = direction
        };

        SendPacket(packet);
    }

    public void WriteEventStop(Vector2 position, Vector2 velocity)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventStop, myId, position, velocity);
        packet.Event.StopEvent = new EventStop
        {

        };

        SendPacket(packet);
    }

    public void WriteEventGetHit(Vector2 position, Vector2 velocity, DamageInfo info)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventHit, myId, position, velocity);
        packet.Event.HitEvent = new EventHit
        {
            Attacker = info.AttackerId,
            ImpactAngle = info.ImpactAngle,
            Impact = info.Impact,
            Damage = info.Damage
        };

        SendPacket(packet);
    }

    public void WriteEventJump(Vector2 position, Vector2 velocity)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventJump, myId, position, velocity);
        packet.Event.JumpEvent = new EventJump
        {

        };

        SendPacket(packet);
    }

    public void WriteEventShoot(Vector2 position, Vector2 velocity, DamageInfo info)
    {
        var packet = CreateCommonEventPacket(GameEvent.Types.action.EventShoot, myId, position, velocity);
        packet.Event.ShootEvent = new EventShoot
        {
            Impact = info.Impact,
            ImpactAngle = info.ImpactAngle,
            Angle = info.shootAngle,
            Damage = info.Damage
        };

        SendPacket(packet);
    }
}
