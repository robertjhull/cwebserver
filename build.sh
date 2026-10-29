#!/bin/bash
cp helloworld.html ./bin/helloworld.html
clang HTTP1.0/server.c -o ./bin/server1.0
clang HTTP1.1/server.c -o ./bin/server1.1