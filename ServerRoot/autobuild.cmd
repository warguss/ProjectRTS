#!/bin/sh

## protobuf lib setup
cd /root/project/ProjectRTS/ServerRoot/server2N/server/lib

ln -s libprotobuf-lite.so.15.0.0 libprotobuf-lite.so
ln -s libprotobuf-lite.so.15.0.0 libprotobuf-lite.so.15

ln -s libprotobuf.so.15.0.0 libprotobuf.so
ln -s libprotobuf.so.15.0.0 libprotobuf.so.15

ln -s libprotoc.so.15.0.0 libprotoc.so
ln -s libprotoc.so.15.0.0 libprotoc.so.15

cd ../src
make clean
make
