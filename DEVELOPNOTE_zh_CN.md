# 开发笔记

## 编译说明

### 上游依赖

目前上游依赖分为两种：一种是直接源码依赖，位于`lib`目录下，这类依赖库被本项目深度修改，以适应本项目的需求，与上游库同步时需要手动合并修改，但在编译时不需要额外的编译步骤；另一种是编译依赖，位于`depend`目录下，这类依赖库基本上是直接从上游库中提取出来的，与上游库同步时只需要重新提取即可，但在编译时需要先编译这些依赖库为动态链接库，再编译本项目。

#### depend目录下的依赖库

- fcitx-qt5-1.2.7.tar.gz

    linux下的输入法框架fcitx的Qt平台支持库，通过`build_fcitx_qt6_linux.sh`脚本编译，编译后将`libfcitxplatforminputcontextplugin-qt6.so`打包在`plugins/platforminputcontexts`目录下，如不需要fcitx支持则无需编译。非linux平台下亦无需编译。

- openssl-3.1.4.tar.gz和libssh2-1.11.0.tar.gz

    用于支持ssh协议的会话，linux通过`build_libssh2.sh`脚本编译，windows mingw通过`build_libssh2_mingw.sh`脚本编译，windows msvc通过`build_libssh2_msvc.bat`脚本编译，编译后需要将安装目录配置到lib/qtssh/qtssh.pri的LIBSSH2_DIR变量中，再编译本项目。

- winpty-0.4.3.tar.gz

    用于支持windows下的伪终端，windows通过`build_winpty_ms.sh`脚本编译，编译后需要将安装目录配置到lib/ptyqt/ptyqt.pri的WINPTY_DIR变量中，再编译本项目。非windows平台下无需编译。

### 编译步骤

完成上游依赖的编译后，即可编译本项目。

如果您选择通过Qt Creator打开`quardCRT.pro`文件，请先选择Tools--External--Linguist--lrelease，生成编译所需的本地化翻译文件，然后点击构建按钮进行编译调试。

您也可以使用项目中预置的编译脚本进行编译，使用编译脚本可以直接编译输出打包好的安装包文件。

- windows（mingw）

    修改build_setup.bat中QT_DIR、QT_TOOLS_DIR、INNO_SETUP_DIR变量的值，然后运行build_setup.bat脚本即可。输出quardCRT_setup.exe安装包。

- windows（msvc）
    
    修改build_setup_msvc.bat中QT_DIR、QT_TOOLS_DIR、INNO_SETUP_DIR变量的值，然后运行build_setup.bat脚本即可。 输出quardCRT_setup.exe安装包。  

- linux（deb）

    修改build_deb.sh中QT_DIR变量的值，然后运行build_deb.sh脚本即可。输出quardCRT.deb安装包。

- linux（AppImage）

    修改build_appimage.sh中QT_DIR变量的值，然后运行build_appimage.sh脚本即可。输出quardCRT.AppImage安装包。

- macos

    修改build_dmg.sh中QT_DIR变量的值，然后运行build_dmg.sh脚本即可。输出quardCRT.dmg安装包。
    