using UnityEngine;
using System.Collections;
using Google.Protobuf;
using System.IO;
using System.Net.Sockets;

public class test : MonoBehaviour
{
    TcpClient client;
    NetworkStream ns;

    byte[] msgBuffer = new byte[8];

    // Use this for initialization
    void Start()
    {
        
        client = new TcpClient("192.168.0.4", 10001);
        ns = client.GetStream();

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
            Google.Protobuf.Examples.AddressBook.Person asd = new Google.Protobuf.Examples.AddressBook.Person
            {
                Id = 3,
                Name = "test"
            };

            byte[] writeBuffer;
            using (MemoryStream ms = new MemoryStream())
            {
                asd.WriteTo(ms);
                writeBuffer = ms.ToArray();
            }

            Debug.Log("msg Length : " + writeBuffer.Length);
            ns.Write(writeBuffer, 0, writeBuffer.Length);
        }
    }

    void DataReceive(System.IAsyncResult ar)
    {
        var parsed = Google.Protobuf.Examples.AddressBook.Person.Parser.ParseFrom(msgBuffer);
        Debug.Log(parsed.Name);

        msgBuffer.Initialize();
    }
}
