using System;
using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class Packet {

    public const int HEADER_LENGTH = 4;

    public UInt32 BodyLength { get; private set; }

    public byte[] RawData { get; private set; }

    public Packet()
    {
        BodyLength = 0;
    }

    public bool AllocateRawData(int BodySize)
    {
        try
        {
            RawData = new byte[HEADER_LENGTH + BodySize];
            BodyLength = (UInt32)BodySize;
            return true;
        }
        catch(Exception e)
        {
            Debug.Log("AllocateRawData Error : " + e.Message);
            return false;
        }
    }

    public bool Encode_Header()
    {
        if(BodyLength == 0)
        {
            Debug.Log("Encode Header Error : BodyLength = 0");
            return false;
        }
        else
        {
            try
            {
                UInt32 temp = BodyLength;
                RawData[0] = (byte)(temp % 256);
                temp /= 256;
                RawData[1] = (byte)(temp % 256);
                temp /= 256; 
                RawData[2] = (byte)(temp % 256);
                temp /= 256;
                RawData[3] = (byte)temp;
                return true;
            }
            catch(Exception e)
            {
                Debug.Log("Encode Header Error : " + e.Message);
                return false;
            }
        }
    }

    public bool Decode_Header()
    {
        try
        {
            UInt32 decoded;
            decoded = (UInt32)RawData[0] + (UInt32)RawData[1] * 256 + (UInt32)RawData[2] * 256 * 256 + (UInt32)RawData[3] * 256 * 256 * 256;
            BodyLength = decoded;
            return true;
        }
        catch(Exception e)
        {
            Debug.Log("Decode Header Error : " + e.Message);
            return false;
        }
    }

    public void Clean()
    {
        BodyLength = 0;
    }
 }

