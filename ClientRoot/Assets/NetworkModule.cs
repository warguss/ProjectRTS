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

    Queue<Packet> packetQueue;

    int testCount = 0;

    // Use this for initialization
    void Start()
    {
        instance = this;

        readPacket = new Packet();
        writePacket = new Packet();
        packetQueue = new Queue<Packet>();
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
                    //ns.BeginRead(msgBuffer, 0, msgBuffer.Length, DataReceive, ns);
                    readPacket.AllocateRawData(0);
                    ns.Read(readPacket.RawData, 0, Packet.HEADER_LENGTH);
                    if (readPacket.Decode_Header())
                    {
                        byte[] packetBodyByteArray;
                        packetBodyByteArray = new byte[(int)readPacket.BodyLength];
                        ns.Read(packetBodyByteArray, 0, (int)readPacket.BodyLength);
                        //packetQueue.Enqueue(readPacket);

                        Debug.Log("Decode_Header : " + readPacket.BodyLength);
                        var parsed = PacketBody.Parser.ParseFrom(packetBodyByteArray);
                        ProcessPacketBody(parsed);
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
        if (client == null)
        {
            TestUI.Instance.PrintText("Trying to connect " + ip + ":" + port);
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

            //byte[] writeBuffer2;
            //writeBuffer2 = eventPacket.ToByteArray();
            //writePacket.AllocateRawData(writeBuffer2.Length);
            //writeBuffer2.CopyTo(writePacket.RawData, Packet.HEADER_LENGTH);
            //writePacket.Encode_Header();

            //ns.Write(writePacket.RawData, 0, writePacket.RawData.Length);
            //TestUI.Instance.PrintText("sended eventPacket , XY = " + testCount + " Body Length : " + writePacket.BodyLength + "Total Length : " + writePacket.RawData.Length);

            //writePacket.Clean();
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText("Send Failed : " + e.Message + "\n" + e.StackTrace);
        }

        testCount++;
    }

    //void DataReceive(System.IAsyncResult ar)
    //{
    //    TestUI.Instance.PrintText("DataReceive()");
    //    var parsed = PacketBody.Parser.ParseFrom(msgBuffer);
    //    ProcessPacketBody(parsed);
    //}

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

    void ProcessPacketBody(PacketBody packetBody)
    {
        string message = "";
        switch (packetBody.MsgType)
        {
            case PacketBody.Types.messageType.UserConnection: 
                message = "UserConnection Data Received. id=" + packetBody.Connect.Id;
                break;
            case PacketBody.Types.messageType.GameEvent:
                message = "GameEvent Data Received. position=" + packetBody.Event.EventPositionX + " , " + packetBody.Event.EventPositionY;
                break;
            default:
                message = "UnknownType : " + (int)packetBody.MsgType;
                break;
        }
        Debug.Log(message);
        TestUI.Instance.PrintText(message);
    }
}

// 메시지 종류 구분 방법
// 읽은 메시지 길이로 바이트 배열 길이 결정