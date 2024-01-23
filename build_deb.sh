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
./tools/linuxdeploy-x86_64.AppImage --executable=./build_release/out/quardCRT --appdir=./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt --plugin=qt
rm -rf ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/apprun-hooks
mv ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/usr ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT
mv ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/bin/quardCRT ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/quardCRT
mv ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/bin/qt.conf ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/qt.conf
cp ./depend/libfcitxplatforminputcontextplugin-qt6.so ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/plugins/platforminputcontexts/libfcitxplatforminputcontextplugin-qt6.so
rm -rf ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/bin
sed -i "s/Prefix = ..\//Prefix = .\//g" ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/qt.conf
chrpath -r "\$ORIGIN/./lib" ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/quardCRT
rm -rf ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/share
cp ./icons/ico.png ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/quardCRT.png
mkdir -p ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT
# 配置打包信息
sed -i "s/#VERSION#/$QUARDCRT_MAJARVERSION.$QUARDCRT_SUBVERSION$QUARDCRT_REVISION/g" ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/DEBIAN/control
cd ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64 || exit
SIZE=$(du -sh -B 1024 ./ | sed "s/.\///g")
cd -
InstalledSize=$SIZE
sed -i "s/#SIZE#/$InstalledSize/g" ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/DEBIAN/control
chmod 755 ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/* -R
mkdir -p ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/plugins/QuardCRT
if [ -d "./prebuilt_plugins" ]; then
    cp ./prebuilt_plugins/*.so ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64/opt/quardCRT/plugins/QuardCRT/
fi
# 打包
dpkg -b ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64 ./dpkg/quardCRT_Linux_"$QUARDCRT_VERSION"_x86_64.deb
echo build success!
###############################################################################
