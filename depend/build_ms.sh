#!/bin/bash
set -e

SHELL_FOLDER=$(cd "$(dirname "$0")";pwd)
tar -xzvf winpty-0.4.3.tar.gz
cd $SHELL_FOLDER/winpty-0.4.3
export PATH=/c/ProgramData/Chocolatey/lib/mingw/tools/install/mingw64/bin:$PATH
./configure
make -j4
make install
mkdir /d/winpty
mkdir /d/winpty/include
cp /usr/local/lib/libwinpty.a /d/winpty/
cp -r /usr/local/include/winpty/* /d/winpty/include/
rm -rf $SHELL_FOLDER/winpty-0.4.3
