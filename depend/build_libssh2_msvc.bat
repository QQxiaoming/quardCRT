#!/bin/bash
set -e

tar -xzvf libssh2-1.11.0.tar.gz
mkdir output
cd ./libssh2-1.11.0
cmake -DCRYPTO_BACKEND=WinCNG -DBUILD_SHARED_LIBS=ON -DCMAKE_INSTALL_PREFIX=../output .
cmake --build . --config Release --target install
