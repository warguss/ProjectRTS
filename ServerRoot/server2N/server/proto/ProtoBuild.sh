#!/bin/bash
export LD_LIBRARY_PATH=../lib

rm -f *.pb.*
rm -f *../../../../ClientRoot/proto/*.cs

echo " ----------------------- CSharp Proto Buffer Message Create -----------------------"
./protoc -I=./ --csharp_out=../../../../ClientRoot/proto ./gameContent.proto


echo " ----------------------- Proto Buffer Message Create -----------------------"
./protoc -I=./ --cpp_out=./ ./gameContent.proto
