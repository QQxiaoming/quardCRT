# Development Note

## Build Instructions

### Upstream Dependencies

Currently there are two types of upstream dependencies: one is direct source code dependency, located in the `lib` directory, these dependencies have been deeply modified to meet the needs of this project, and need to be manually merged when synchronizing with the upstream library, but no additional compilation steps are required during compilation; the other is a compilation dependency, located in the `depend` directory, these dependencies are basically extracted directly from the upstream library, and only need to be re-extracted when synchronizing with the upstream library, but need to be compiled into dynamic link libraries before compiling this project.

#### Dependencies under the `depend` directory

- fcitx-qt5-1.2.7.tar.gz

    Qt platform support library for the fcitx input method framework under linux, compiled using the `build_fcitx_qt6_linux.sh` script, and the `libfcitxplatforminputcontextplugin-qt6.so` is packaged in the `plugins/platforminputcontexts` directory after compilation. If fcitx support is not needed, no compilation is required. No compilation is required for non-linux platforms.

- openssl-3.1.4.tar.gz和libssh2-1.11.0.tar.gz

    Used to support ssh protocol sessions, linux is compiled using the `build_libssh2.sh` script, windows mingw is compiled using the `build_libssh2_mingw.sh` script, and windows msvc is compiled using the `build_libssh2_msvc.bat` script. After compilation, the installation directory needs to be configured to the LIBSSH2_DIR variable in lib/qtssh/qtssh.pri, and then compile this project.

- winpty-0.4.3.tar.gz

    Used to support pseudo terminals under windows(Mingw), compiled using the `build_winpty_ms.sh` script under windows, after compilation, the installation directory needs to be configured to the WINPTY_DIR variable in lib/ptyqt/ptyqt.pri, and then compile this project. No compilation is required for MSVC environment and non-windows platforms.

### Compilation Steps

After completing the compilation of the upstream dependencies, you can compile this project.

If you choose to open the `quardCRT.pro` file through Qt Creator, please first select Tools--External--Linguist--lrelease, generate the localized translation files required for compilation, and then click the build button to compile and debug.

You can also use the pre-set compilation scripts in the project to compile. Using the compilation scripts can directly compile and output the packaged installation package files.

- windows（mingw）

    Modify the values of the QT_DIR, QT_TOOLS_DIR, and INNO_SETUP_DIR variables in build_setup.bat, and then run the build_setup.bat script. The quardCRT_setup.exe installation package is output.

- windows（msvc）
    
    Modify the values of the QT_DIR, QT_TOOLS_DIR, and INNO_SETUP_DIR variables in build_setup_msvc.bat, and then run the build_setup.bat script. The quardCRT_setup.exe installation package is output.

- linux（deb）

    Modify the value of the QT_DIR variable in build_deb.sh, and then run the build_deb.sh script. The quardCRT.deb installation package is output.

- linux（AppImage）

    Modify the value of the QT_DIR variable in build_appimage.sh, and then run the build_appimage.sh script. The quardCRT.AppImage installation package is output.

- macos

    Modify the value of the QT_DIR variable in build_dmg.sh, and then run the build_dmg.sh script. The quardCRT.dmg installation package is output.
    