@echo off
set "FROST_VS="
for /f "usebackq tokens=*" %%i in (`"%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "FROST_VS=%%i"
if not defined FROST_VS (
  echo Install Visual Studio C++ Desktop development tools first.
  exit /b 1
)
call "%FROST_VS%\VC\Auxiliary\Build\vcvars32.bat" >nul
exit /b %errorlevel%
