using UnityEngine;
using System.Collections;
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
    byte[] msgBuffer = new byte[8];

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
            TestUI.Instance.PrintText("Sending AddressBook.Person... Id=3, Name=test");
            Google.Protobuf.Examples.AddressBook.Person person = new Google.Protobuf.Examples.AddressBook.Person
            {
                Id = 3,
                Name = "test"
            };
        
            byte[] writeBuffer;
            using (MemoryStream ms = new MemoryStream())
            {
                person.WriteTo(ms);
                writeBuffer = ms.ToArray();
            }

            try
            {
                ns.Write(writeBuffer, 0, writeBuffer.Length);
                TestUI.Instance.PrintText("sended msg Length : " + writeBuffer.Length);
            }
            catch (Exception e)
            {
                TestUI.Instance.PrintText("Send Failed : " + e.Message);
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
                TestUI.Instance.PrintText("Connected");
            }
            catch (Exception e)
            {
                TestUI.Instance.PrintText(e.Message);
            }
        }
    }

    void DataReceive(System.IAsyncResult ar)
    {
        var parsed = Google.Protobuf.Examples.AddressBook.Person.Parser.ParseFrom(msgBuffer);
        string message = "Data Received. Name=" + parsed.Name + ", id=" + parsed.Id;
        Debug.Log(message);
        TestUI.Instance.PrintText(message);

        msgBuffer.Initialize();
    }
}

// 메시지 종류 구분 방법
// 읽은 메시지 길이로 바이트 배열 길이 결정