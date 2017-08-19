using UnityEngine;
using System.Collections;
using Google.Protobuf;
using System.IO;

public class test : MonoBehaviour {
	// Use this for initialization
	void Start () {
        Google.Protobuf.Examples.AddressBook.Person asd = new Google.Protobuf.Examples.AddressBook.Person
        {
                Id = 3,
                Name = "test"
        };

        byte[] bmsg;
        using (MemoryStream ms = new MemoryStream())
        {
            asd.WriteTo(ms);
            bmsg = ms.ToArray();
        }

        var parsed = Google.Protobuf.Examples.AddressBook.Person.Parser.ParseFrom(bmsg);
        Debug.Log(parsed.Name);
	}
	
	// Update is called once per frame
	void Update () {
	
	}
}
