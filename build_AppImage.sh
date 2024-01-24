#!/bin/sh

###############################################################################
# 定义Qt目录
QT_DIR=/opt/Qt6.2.0/6.2.0/gcc_64
###############################################################################


###############################################################################
# 定义版本号
QUARDCRT_MAJARVERSION=$(< ./version.txt cut -d '.' -f 1)
QUARDCRT_SUBVERSION=$(<  ./version.txt cut -d '.' -f 2)
QUARDCRT_REVISION=$(< ./version.txt cut -d '.' -f 3)
export PATH=$QT_DIR/bin:$PATH
export LD_LIBRARY_PATH=$QT_DIR/lib:$LD_LIBRARY_PATH
export QT_PLUGIN_PATH=$QT_DIR/plugins
export QML2_IMPORT_PATH=$QT_DIR/qml
# 合成版本号
QUARDCRT_VERSION="V"$QUARDCRT_MAJARVERSION$QUARDCRT_SUBVERSION$QUARDCRT_REVISION
# 编译
rm -rf .qmake.stash Makefile
lrelease ./quardCRT.pro
qmake ./quardCRT.pro -spec linux-g++ CONFIG+=qtquickcompiler
make clean
make -j8 
# clean打包目录
rm -rf ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64 
rm -f ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64.deb
# 构建打包目录
cp -r ./dpkg/quardCRT ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64
# 使用linuxdeployqt拷贝依赖so库到打包目录
export QMAKE=$QT_DIR/bin/qmake
mkdir -p ./AppDir/usr/plugins/QuardCRT
if [ -d "./prebuilt_plugins" ]; then
    cp ./prebuilt_plugins/*.so ./AppDir/usr/plugins/QuardCRT/
fi
./tools/linuxdeploy-x86_64.AppImage --executable=./build_release/out/quardCRT --appdir=./AppDir --icon-file=./icons/ico.svg --desktop-file=./AppDirRes/quardCRT.desktop --plugin=qt --output appimage
mkdir AppImageOut
cp ./quardCRT-*.AppImage ./AppImageOut/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64.AppImage
