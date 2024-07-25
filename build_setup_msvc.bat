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
qmake quardCRT.pro -tp vc
msbuild quardCRT.vcxproj /p:Configuration=Release /p:Platform=x64 /p:BuildInParallel=false
:: clean打包目录
if exist ".\InnoSetup\build" (
    rmdir /Q /S .\InnoSetup\build
)
:: 配置打包信息
copy /y .\InnoSetup\build_setup.iss .\InnoSetup\build_temp_setup.iss
.\tools\sed\sed.exe -i "s/#VERSION#/%QUARDCRT_VERSION%/g" .\InnoSetup\build_temp_setup.iss
.\tools\sed\sed.exe -i "s/#VERSIONINFOVERSION#/%QUARDCRT_VERSION%.000/g" .\InnoSetup\build_temp_setup.iss
del /f /q /a .\sed*
:: 构建打包目录
xcopy /y .\build_release\out\quardCRT.exe .\InnoSetup\build\
:: 使用windeployqt拷贝依赖dll库到打包目录
windeployqt --dir .\InnoSetup\build .\InnoSetup\build\quardCRT.exe
xcopy /y "%Python3_ROOT_DIR%\python311.dll" ".\InnoSetup\build\"
xcopy /y "%Python3_ROOT_DIR%\Lib" ".\InnoSetup\build\pythonlib\lib" /E /I
xcopy /y "D:\libssh2\bin\*.dll" ".\InnoSetup\build\"
xcopy /y .\scripts\Profile.ps1 .\InnoSetup\build\
xcopy /y .\font\inziu-iosevkaCC-SC-regular.ttf .\InnoSetup\build\
mkdir ".\InnoSetup\build\plugins"
mkdir ".\InnoSetup\build\plugins\QuardCRT"
:: 判断是否存在prebuilt_plugins目录
if exist ".\prebuilt_plugins" (
    xcopy /y .\prebuilt_plugins\*.dll .\InnoSetup\build\plugins\QuardCRT\
)
:: 打包
echo "wait inno build setup..."
iscc /q ".\InnoSetup\build_temp_setup.iss"
del .\InnoSetup\build_temp_setup.iss
FOR /F "delims=. tokens=1-3" %%x IN ("%QUARDCRT_VERSION%") DO (
    set "QUARDCRT_MAJARVERSION=%%x"
    set "QUARDCRT_SUBVERSION=%%y"
    set "QUARDCRT_REVISION=%%z"
)
mkdir ".\output"
echo F|xcopy /S /Q /Y /F ".\InnoSetup\quardCRT_setup.exe" ".\output\quardCRT_windows_V%QUARDCRT_MAJARVERSION%%QUARDCRT_SUBVERSION%%QUARDCRT_REVISION%_x86_64_msvc_setup.exe"
echo "build success!"
:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::
