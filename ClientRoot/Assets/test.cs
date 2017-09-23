using UnityEngine;
using System.Collections;
using Google.Protobuf;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;
using RTS;

public class test : MonoBehaviour
{
    public GameObject PlayerPrefab;

    TcpClient client;
    NetworkStream ns;
    Dictionary<int, MainCharacter> playerCharacters;
    int myId;
    int testId = 2;

    byte[] msgBuffer = new byte[256];

    // Use this for initialization
    void Start()
    {
        myId = 1;
        testId = 2;
        playerCharacters = new Dictionary<int, MainCharacter>();
        playerCharacters.Add(myId, Instantiate(PlayerPrefab).GetComponent<MainCharacter>());
        playerCharacters.Add(testId, Instantiate(PlayerPrefab).GetComponent<MainCharacter>());
//        client = new TcpClient("192.168.0.4", 10001);
//        ns = client.GetStream();

    }
	
    // Update is called once per frame
    void Update()
    {
//        if (client != null)
//        {
//            if (ns.DataAvailable)
//            {
//                ns.BeginRead(msgBuffer, 0, msgBuffer.Length, DataReceive, ns);
//            }
//            else
//            {
//                
//            }
//        }

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

        ProcessInput();
    }

    void FixedUpdate()
    {
    }

    void userJoin(int id)
    {
    }

    void userLeave(int id)
    {
    }

    void DataReceive(System.IAsyncResult ar)
    {
        var parsed = Google.Protobuf.Examples.AddressBook.Person.Parser.ParseFrom(msgBuffer);
        Debug.Log(parsed.Name);

        msgBuffer.Initialize();
    }

    void SendInputToCharacter(int player, PlayerAction action)
    {
        playerCharacters[player].SetInput(action);
    }

    void ProcessInput()
    {
        if (Input.GetKeyDown(KeyCode.LeftArrow))
            SendInputToCharacter(myId, PlayerAction.Left);
        else if (Input.GetKeyDown(KeyCode.RightArrow))
            SendInputToCharacter(myId, PlayerAction.Right);
        else if(Input.GetKeyUp(KeyCode.LeftArrow) || Input.GetKeyUp(KeyCode.RightArrow))
            SendInputToCharacter(myId, PlayerAction.Stop);
        
        if (Input.GetKeyDown(KeyCode.UpArrow))
            SendInputToCharacter(myId, PlayerAction.Jump);

        if (Input.GetKeyDown(KeyCode.A))
            SendInputToCharacter(testId, PlayerAction.Left);
        else if (Input.GetKeyDown(KeyCode.D))
            SendInputToCharacter(testId, PlayerAction.Right);
        else if(Input.GetKeyUp(KeyCode.A) || Input.GetKeyUp(KeyCode.D))
            SendInputToCharacter(testId, PlayerAction.Stop);

        if (Input.GetKeyDown(KeyCode.W))
            SendInputToCharacter(testId, PlayerAction.Jump);
        
    }
}

// 메시지 종류 구분 방법
// 읽은 메시지 길이로 바이트 배열 길이 결정