using UnityEngine;
using System.Collections;
using Server2N;
using Google.Protobuf;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;
using System;

public class NetworkModule : MonoBehaviour
{
    public static NetworkModule instance;

    TcpClient client;
    NetworkStream ns;
    byte[] msgBuffer = new byte[256];
    int testCount = 0;

    // Use this for initialization
    void Start()
    {
        instance = this;
    }
	
    // Update is called once per frame
    void Update()
    {
        if (client != null)
        {
            if (ns.DataAvailable)
            {
                ns.BeginRead(msgBuffer, 0, msgBuffer.Length, DataReceive, ns);
            }
            else
            {
                        
            }
        }

        if (Input.GetKeyDown(KeyCode.Space))
        {
            Send();
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
            using (MemoryStream ms = new MemoryStream())
            {
                byte[] writeBuffer;
                connectionPacket.WriteTo(ms);
                writeBuffer = ms.ToArray();

                //ns.Write(writeBuffer, 0, writeBuffer.Length);
                TestUI.Instance.PrintText("sended connectionPacket, id = " + testCount + " Length : " + writeBuffer.Length);
            }

            using (MemoryStream ms = new MemoryStream())
            {
                byte[] writeBuffer;
                eventPacket.WriteTo(ms);
                writeBuffer = ms.ToArray();

                //ns.Write(writeBuffer, 0, writeBuffer.Length);
                TestUI.Instance.PrintText("sended eventPacket , XY = " + testCount + " Length : " + writeBuffer.Length);
            }
        }
        catch (Exception e)
        {
            TestUI.Instance.PrintText("Send Failed : " + e.Message);
        }

        testCount++;
    }

    void DataReceive(System.IAsyncResult ar)
    {
        var parsed = PacketBody.Parser.ParseFrom(msgBuffer);
        ProcessPacketBody(parsed);
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

    void ProcessPacketBody(PacketBody packetBody)
    {
        string message = "";
        switch (packetBody.MsgType)
        {
            case PacketBody.Types.messageType.UserConnection: 
                message = "UserConnection Data Received. id=" + packetBody.Connect.Id;
                break;
            case PacketBody.Types.messageType.GameEvent:
                message = "GameEvent Data Received. position=" + packetBody.Event.EventPositionX;
                break;
        }
        Debug.Log(message);
        TestUI.Instance.PrintText(message);

        msgBuffer.Initialize();
    }
}

// 메시지 종류 구분 방법
// 읽은 메시지 길이로 바이트 배열 길이 결정