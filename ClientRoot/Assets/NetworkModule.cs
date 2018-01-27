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

                        //Debug.Log("Decode_Header : " + readPacket.BodyLength);
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

    public void Connect(string ip, int port)
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
            WriteTryConnection();
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

    PacketBody CreateConnectionPacket(UserConnection.Types.ConnectionType type, int id)
    {
        UserConnection uesrConnection = new UserConnection
        {
            ConType = type,
            ConnectorId = id
        };

        PacketBody packetBody = new PacketBody
        {
            MsgType = PacketBody.Types.messageType.UserConnection,
            Connect = uesrConnection,
        };

        return packetBody;
    }

    PacketBody CreateEventPacket(GameEvent.Types.action type, int id, Vector2 position, Vector2 velocity, int actionProperty = 0)
    {
        GameEvent gameEvent = new GameEvent
        {
            Act = GameEvent.Types.action.Move,
            EventPositionX = position.x,
            EventPositionY = position.y,
            ActionProperty = actionProperty,
            InvokerId = id,
        };

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

    public void WriteTryConnection()
    {
        var packet = CreateConnectionPacket(UserConnection.Types.ConnectionType.TryConnect, 0);

        SendPacket(packet);
    }

    public void WriteEventMove(Vector2 position, Vector2 velocity, bool isLeft)
    {
        int property = 0;
        if (!isLeft)
            property = 1;

        var packet = CreateEventPacket(GameEvent.Types.action.Move, myId, position, velocity, property);

        SendPacket(packet);
    }

    public void WriteEventStop(Vector2 position, Vector2 velocity)
    {
        var packet = CreateEventPacket(GameEvent.Types.action.Stop, myId, position, velocity);

        SendPacket(packet);
    }

    public void WriteEventGetHit(Vector2 position, Vector2 velocity, int damage, int hitRecovery, int impact, int impactAngle)
    {
        ////////////////////////////////////
        var packet = CreateEventPacket(GameEvent.Types.action.GetHit, myId, position, velocity);

        SendPacket(packet);
    }

    public void PlayerEventJump(Vector2 position, Vector2 velocity)
    {
        var packet = CreateEventPacket(GameEvent.Types.action.Jump, myId, position, velocity);

        SendPacket(packet);
    }

    public void PlayerEventShoot(Vector2 position, Vector2 velocity, bool isLeft)
    {
        var packet = CreateEventPacket(GameEvent.Types.action.Shoot, myId, position, velocity);

        SendPacket(packet);
    }
}
