#!/bin/sh

###############################################################################
# 定义Qt目录
QT_DIR=/opt/Qt6.2.0/6.2.0/gcc_64
###############################################################################


###############################################################################
# 定义版本号
QUARDCRT_MAJARVERSION=$(< ./version.txt | cut -d '.' -f 1)
QUARDCRT_SUBVERSION=$(< ./version.txt | cut -d '.' -f 2)
QUARDCRT_REVISION=$(< ./version.txt | cut -d '.' -f 3)
export PATH=$QT_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_DIR/lib:$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=$QT_DIR/plugins
export QML2_IMPORT_PATH=$QT_DIR/qml
# 合成版本号
QUARDCRT_VERSION="V"$QUARDCRT_MAJARVERSION$QUARDCRT_SUBVERSION$QUARDCRT_REVISION
# 编译
rm -rf .qmake.stash Makefile
$QT_DIR/bin/lrelease ./quardCRT.pro
$QT_DIR/bin/qmake -makefile
make
cp ./tools/create-dmg/build-dmg.sh ./build_release/out/build-dmg.sh
cp ./tools/create-dmg/installer_background.png ./build_release/out/installer_background.png
cd ./build_release/out
# 打包
$QT_DIR/bin/macdeployqt quardCRT.app
otool -L ./quardCRT.app/Contents/MacOS/quardCRT
./build-dmg.sh quardCRT
echo build success!
###############################################################################
