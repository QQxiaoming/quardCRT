#!/bin/sh

###############################################################################
# 定义Qt目录
QT_DIR=/opt/Qt6.2.0/6.2.0/gcc_64
###############################################################################


###############################################################################
# 定义版本号
QUARDCRT_MAJARVERSION=$(< ./version.txt cut -d '.' -f 1)
QUARDCRT_SUBVERSION=$(< ./version.txt cut -d '.' -f 2)
QUARDCRT_REVISION=$(< ./version.txt cut -d '.' -f 3)
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
mkdir -p ./quardCRT.app/Contents/Translations
cp $QT_DIR/translations/qt_*.qm ./quardCRT.app/Contents/Translations/
cp $QT_DIR/translations/qtbase_*.qm ./quardCRT.app/Contents/Translations/
mkdir -p ./quardCRT.app/Contents/MacOS/plugins/QuardCRT
if [ -d "../../prebuilt_plugins" ]; then
    cp ../../prebuilt_plugins/*.dylib ./quardCRT.app/Contents/MacOS/plugins/QuardCRT/
fi
python_stdlib=$(python3 -c "import sysconfig; print(sysconfig.get_path('stdlib'))")
mkdir -p ./quardCRT.app/Contents/Frameworks/pythonlib/lib
cp -r $python_stdlib/../../lib/python3.11 ./quardCRT.app/Contents/Frameworks/pythonlib/lib/
cp -r $python_stdlib/../../lib/libpython3.11.dylib ./quardCRT.app/Contents/Frameworks/pythonlib/lib/
install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.11/Python @executable_path/../Frameworks/pythonlib/lib/libpython3.11.dylib ./quardCRT.app/Contents/MacOS/quardCRT 
install_name_tool -change /Library/Frameworks/Python.framework/Versions/3.11/Python @executable_path/../pythonlib/libpython3.11.dylib ./quardCRT.app/Contents/Frameworks/pythonlib/lib/libpython3.11.dylib
codesign --force --deep --sign - ./quardCRT.app/Contents/Frameworks/pythonlib/lib/libpython3.11.dylib
sudo ./build-dmg.sh quardCRT
cd ../../
mkdir dmgOut
cpu=$(sysctl -n machdep.cpu.brand_string)
ARCH="x86_64"
case $cpu in
  *Intel*) ARCH="x86_64" ;;
  *Apple*) ARCH="arm64" ;;
esac
cp ./build_release/out/quardCRT.dmg ./dmgOut/quardCRT_macos_"$QUARDCRT_VERSION"_"$ARCH".dmg
echo build success!
###############################################################################
