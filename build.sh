#!/bin/bash
cp helloworld.html ./bin/helloworld.html
clang HTTP1.1/server.c -o ./bin/server1.1
clang HTTP2/server.c -o ./bin/server2