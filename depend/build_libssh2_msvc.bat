@echo off
setlocal

cd /d "%~dp0"
set "ROOT=%CD%"

if exist output rmdir /s /q output
if exist libssh2-1.11.0 rmdir /s /q libssh2-1.11.0

tar -xzvf libssh2-1.11.0.tar.gz
mkdir output
cd /d "%ROOT%\libssh2-1.11.0"

cmake -S . -B build-release -G "NMake Makefiles" ^
	-DCRYPTO_BACKEND=WinCNG ^
	-DBUILD_SHARED_LIBS=ON ^
	-DBUILD_STATIC_LIBS=ON ^
	-DBUILD_EXAMPLES=OFF ^
	-DBUILD_TESTING=OFF ^
	-DCMAKE_BUILD_TYPE=Release ^
	-DCMAKE_MSVC_RUNTIME_LIBRARY=MultiThreadedDLL ^
	-DCMAKE_INSTALL_PREFIX="%ROOT%\output"
if errorlevel 1 exit /b 1

cmake --build build-release --target install
if errorlevel 1 exit /b 1
