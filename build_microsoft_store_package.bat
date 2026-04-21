@echo off

:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 定义Qt目录
set "QT_DIR=C:/Qt/Qt6.2.0/6.2.0/msvc64/bin"
set "QT_TOOLS_DIR=C:/Qt/Qt6.2.0/Tools/msvc64/bin"
:: 定义Inno Setup目录
set "INNO_SETUP_DIR=C:/Program Files (x86)/Inno Setup 6"
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::


:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
:: 定义版本号
set /p QUARDCRT_VERSION=<version.txt
:: 设置环境变量
set "PATH=%QT_DIR%;%QT_TOOLS_DIR%;%INNO_SETUP_DIR%;%PATH%"
:: 编译
del .qmake.stash Makefile
if exist ".\build_debug" (
    rmdir /Q /S .\build_debug
)
if exist ".\build_release" (
    rmdir /Q /S .\build_release
)
lrelease quardCRT.pro
qmake quardCRT.pro -tp vc CONFIG+=microsoft_store_build
msbuild quardCRT.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildInParallel=false
:: clean打包目录
mkdir ".\package"
:: 构建打包目录
xcopy /y .\build_release\out\quardCRT.exe .\package\
:: 使用windeployqt拷贝依赖dll库到打包目录
windeployqt --dir .\package .\package\quardCRT.exe
xcopy /y "%Python3_ROOT_DIR%\python311.dll" ".\package\"
xcopy /y "%Python3_ROOT_DIR%\Lib" ".\package\pythonlib\lib" /E /I
xcopy /y "D:\libssh2\bin\*.dll" ".\package\"
xcopy /y .\scripts\Profile.ps1 .\package\
xcopy /y .\font\inziu-iosevkaCC-SC-regular.ttf .\package\
mkdir ".\package\plugins"
mkdir ".\package\plugins\QuardCRT"
:: 判断是否存在prebuilt_plugins目录
if exist ".\prebuilt_plugins" (
    xcopy /y .\prebuilt_plugins\*.dll .\package\plugins\QuardCRT\
)
echo "build success!"
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
