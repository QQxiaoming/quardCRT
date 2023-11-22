#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)

cd $SHELL_FOLDER
tar -xzvf openssl-3.1.4.tar.gz
cd $SHELL_FOLDER/openssl-3.1.4
./config --prefix=$SHELL_FOLDER/output --openssldir=$SHELL_FOLDER/output no-threads
make -j16
make install
rm -rf $SHELL_FOLDER/openssl-3.1.4

cd $SHELL_FOLDER
tar -xzvf libssh2-1.11.0.tar.gz
cd $SHELL_FOLDER/libssh2-1.11.0
./configure --prefix=$SHELL_FOLDER/output --enable-shared --disable-static --without-libz --with-openssl=$SHELL_FOLDER/output
make -j16
make install
rm -rf $SHELL_FOLDER/libssh2-1.11.0
