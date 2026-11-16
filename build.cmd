@echo off
setlocal
pushd "%~dp0"
call scripts\build-env.cmd
if errorlevel 1 exit /b 1
set "FROST_CONFIG=Release"
if /i "%~1"=="Debug" set "FROST_CONFIG=Debug"
msbuild "Frost Breaker.sln" /t:Rebuild /p:Configuration=%FROST_CONFIG% /p:Platform=Win32 /verbosity:minimal /nologo
set "FROST_RESULT=%errorlevel%"
popd
exit /b %FROST_RESULT%
