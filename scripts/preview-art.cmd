@echo off
setlocal
pushd "%~dp0.."
call scripts\build-env.cmd
if errorlevel 1 exit /b 1
if not exist build\preview mkdir build\preview
cl /nologo /std:c++17 /EHsc /MD /W4 /WX /Isrc /Ithird_party\SFML-2.6.1\include scripts\PreviewArt.cpp src\Ball.cpp src\Paddle.cpp src\Bricks.cpp src\Constants.cpp src\Logic.cpp src\Levels.cpp src\Design.cpp /Febuild\PreviewArt.exe /Fobuild\preview\ /link /LIBPATH:third_party\SFML-2.6.1\lib sfml-graphics.lib sfml-window.lib sfml-system.lib sfml-audio.lib
if errorlevel 1 exit /b 1
set "PATH=%CD%\bin;%CD%\third_party\SFML-2.6.1\bin;%PATH%"
build\PreviewArt.exe
if errorlevel 1 exit /b 1
python scripts\preview_campaign.py
set "FROST_RESULT=%errorlevel%"
popd
exit /b %FROST_RESULT%
