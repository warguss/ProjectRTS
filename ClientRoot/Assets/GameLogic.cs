using UnityEngine;
using System.Collections;
using Google.Protobuf;
using System.IO;
using System.Net.Sockets;
using System.Collections.Generic;
using RTS;

public class GameLogic : MonoBehaviour
{
    public GameObject PlayerPrefab;
    public CameraMove CameraScript;

    TcpClient client;
    NetworkStream ns;
    Dictionary<int, MainCharacter> playerCharacters;
    int myId = 1; 
    int testId = 2;

    byte[] msgBuffer = new byte[256];

    // Use this for initialization
    void Start()
    {
        playerCharacters = new Dictionary<int, MainCharacter>();
        AddPlayer(myId);
        AddPlayer(testId);

        CameraScript.SetTarget(playerCharacters[myId].gameObject);
    }
	
    // Update is called once per frame
    void Update()
    {
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

    void AddPlayer(int playerId)
    {
        MainCharacter characterScript = Instantiate(PlayerPrefab).GetComponent<MainCharacter>();
        playerCharacters.Add(playerId, characterScript);
        characterScript.playerId = playerId;
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
        if (Input.GetKeyDown(KeyCode.RightShift))
            SendInputToCharacter(myId, PlayerAction.Fire);

        if (Input.GetKeyDown(KeyCode.A))
            SendInputToCharacter(testId, PlayerAction.Left);
        else if (Input.GetKeyDown(KeyCode.D))
            SendInputToCharacter(testId, PlayerAction.Right);
        else if(Input.GetKeyUp(KeyCode.A) || Input.GetKeyUp(KeyCode.D))
            SendInputToCharacter(testId, PlayerAction.Stop);

        if (Input.GetKeyDown(KeyCode.W))
            SendInputToCharacter(testId, PlayerAction.Jump);
        if (Input.GetKeyDown(KeyCode.Q))
            SendInputToCharacter(testId, PlayerAction.Fire);
        
    }
}
   