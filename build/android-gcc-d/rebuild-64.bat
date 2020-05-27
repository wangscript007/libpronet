@echo off
set THIS_DIR=%~sdp0

if exist %THIS_DIR%libs\arm64-v8a      rmdir /s /q %THIS_DIR%libs\arm64-v8a
if exist %THIS_DIR%libs\x86_64         rmdir /s /q %THIS_DIR%libs\x86_64
if exist %THIS_DIR%obj\local\arm64-v8a rmdir /s /q %THIS_DIR%obj\local\arm64-v8a
if exist %THIS_DIR%obj\local\x86_64    rmdir /s /q %THIS_DIR%obj\local\x86_64

@echo on
call ndk-build -B V=1 NDK_DEBUG=1 APP_ABI="arm64-v8a x86_64"
@echo off

copy /y %THIS_DIR%obj\local\arm64-v8a\libmbedtls.a  %THIS_DIR%libs\arm64-v8a\
copy /y %THIS_DIR%obj\local\x86_64\libmbedtls.a     %THIS_DIR%libs\x86_64\

copy /y %THIS_DIR%obj\local\arm64-v8a\libpro_util.a %THIS_DIR%libs\arm64-v8a\
copy /y %THIS_DIR%obj\local\x86_64\libpro_util.a    %THIS_DIR%libs\x86_64\

@echo on
pause
