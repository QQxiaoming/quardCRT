#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
tar -xzvf libssh2-1.11.0.tar.gz
mkdir output
cd $SHELL_FOLDER/libssh2-1.11.0
cmake -DCRYPTO_BACKEND=WinCNG -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=../output --build .
cmake --build . --target install
