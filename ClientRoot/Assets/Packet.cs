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
        if(BodyLength ==0)
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

    public static unsafe void Copy(byte[] source, int sourceOffset, byte[] target,
        int targetOffset, int count)
    {
        // If either array is not instantiated, you cannot complete the copy.
        if ((source == null) || (target == null))
        {
            throw new System.ArgumentException();
        }

        // If either offset, or the number of bytes to copy, is negative, you
        // cannot complete the copy.
        if ((sourceOffset < 0) || (targetOffset < 0) || (count < 0))
        {
            throw new System.ArgumentException();
        }

        // If the number of bytes from the offset to the end of the array is 
        // less than the number of bytes you want to copy, you cannot complete
        // the copy. 
        if ((source.Length - sourceOffset < count) ||
            (target.Length - targetOffset < count))
        {
            throw new System.ArgumentException();
        }

        // The following fixed statement pins the location of the source and
        // target objects in memory so that they will not be moved by garbage
        // collection.
        fixed (byte* pSource = source, pTarget = target)
        {
            // Set the starting points in source and target for the copying.
            byte* ps = pSource + sourceOffset;
            byte* pt = pTarget + targetOffset;

            // Copy the specified number of bytes from source to target.
            for (int i = 0; i < count; i++)
            {
                *pt = *ps;
                pt++;
                ps++;
            }
        }
    }
 }

