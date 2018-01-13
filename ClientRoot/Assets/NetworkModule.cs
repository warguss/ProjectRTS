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

    int testCount = 0;

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
        if (Input.GetKeyDown(KeyCode.Space))
        {
            Send();
        }
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
 
    public void Send()
    {
        TestUI.Instance.PrintText("Sending ConnectionPacket, EventPacket");
        PacketBody connectionPacket = CreateConnectionPacket(testCount);
        PacketBody eventPacket = CreateEventPacket((float)testCount);
     
        try
        {
            byte[] writeBuffer;
            writeBuffer = connectionPacket.ToByteArray();
            writePacket.AllocateRawData(writeBuffer.Length);
            writeBuffer.CopyTo(writePacket.RawData, Packet.HEADER_LENGTH);
            writePacket.Encode_Header();

            ns.Write(writePacket.RawData, 0, writePacket.RawData.Length);
            TestUI.Instance.PrintText("sended connectionPacket, id = " + testCount + " Body Length : " + writePacket.BodyLength + "Total Length : " + writePacket.RawData.Length);

            writePacket.Clean();

            byte[] writeBuffer2;
            writeBuffer2 = eventPacket.ToByteArray();
            writePacket.AllocateRawData(writeBuffer2.Length);
            writeBuffer2.CopyTo(writePacket.RawData, Packet.HEADER_LENGTH);
            writePacket.Encode_Header();

            ns.Write(writePacket.RawData, 0, writePacket.RawData.Length);
            TestUI.Instance.PrintText("sended eventPacket , XY = " + testCount + " Body Length : " + writePacket.BodyLength + "Total Length : " + writePacket.RawData.Length);

            writePacket.Clean();
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText("Send Failed : " + e.Message + "\n" + e.StackTrace);
        }

        testCount++;
    }

    PacketBody CreateConnectionPacket(int test)
    {
        UserConnection uesrConnection = new UserConnection();
        uesrConnection.ConType = UserConnection.Types.ConnectionType.Connect;
        uesrConnection.Id = test;

        PacketBody packetBody = new PacketBody();
        packetBody.MsgType = PacketBody.Types.messageType.UserConnection;
        packetBody.Connect = uesrConnection;

        return packetBody;
    }

    PacketBody CreateEventPacket(float test)
    {
        GameEvent gameEvent = new GameEvent();
        gameEvent.Act = GameEvent.Types.action.Move;
        gameEvent.EventPositionX = test;
        gameEvent.EventPositionY = test;

        PacketBody packetBody = new PacketBody();
        packetBody.MsgType = PacketBody.Types.messageType.GameEvent;
        packetBody.Event = gameEvent;

        return packetBody;
    }

    public PacketBody GetReceivedPacketBody()
    {
        lock (QueueLock)
        {
            return RecevedPacketBodyQueue.Dequeue();
        }
    }
}

// 메시지 종류 구분 방법
// 읽은 메시지 길이로 바이트 배열 길이 결정