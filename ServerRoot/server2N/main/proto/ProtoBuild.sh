#!/bin/bash


./protoc -I=./ --csharp_out=../../../../ClientRoot/proto ./*.proto

./protoc -I=./ --cpp_out=./ ./*.proto
