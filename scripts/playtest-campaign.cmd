@echo off
setlocal
pushd "%~dp0.."
call scripts\build-env.cmd
if errorlevel 1 exit /b 1
if not exist build\playtest mkdir build\playtest
cl /nologo /std:c++17 /O2 /EHsc /MD /W4 /WX /Isrc /Ithird_party\SFML-2.6.1\include scripts\PlaytestCampaign.cpp src\Ball.cpp src\Paddle.cpp src\Bricks.cpp src\Constants.cpp src\Logic.cpp src\Levels.cpp src\Design.cpp /Febuild\PlaytestCampaign.exe /Fobuild\playtest\ /link /LIBPATH:third_party\SFML-2.6.1\lib sfml-graphics.lib sfml-window.lib sfml-system.lib sfml-audio.lib
if errorlevel 1 exit /b 1
set "PATH=%CD%\bin;%CD%\third_party\SFML-2.6.1\bin;%PATH%"
build\PlaytestCampaign.exe
set "FROST_RESULT=%errorlevel%"
popd
exit /b %FROST_RESULT%
