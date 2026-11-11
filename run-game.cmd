@echo off
setlocal EnableExtensions DisableDelayedExpansion
pushd "%~dp0" || exit /b 1

if not "%~1"=="" if /i not "%~1"=="--check" if /i not "%~1"=="--check-gl" (
    echo Usage: run-game.cmd [--check or --check-gl]
    popd
    exit /b 2
)

set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if not exist "%VSWHERE%" (
    echo Visual Studio Installer was not found. Install Visual Studio with Desktop development with C++.
    goto failed
)
set "VSROOT="
for /f "usebackq delims=" %%I in (`"%VSWHERE%" -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath`) do set "VSROOT=%%I"
if not defined VSROOT (
    echo No Visual Studio C++ installation was found. Install the v145 toolset and Windows SDK.
    goto failed
)

echo Building Monster Clash - Debug x64...
"%VSROOT%\MSBuild\Current\Bin\MSBuild.exe" OpenJL.vcxproj /p:Configuration=Debug /p:Platform=x64 /verbosity:minimal /nologo
if errorlevel 1 goto failed

if /i "%~1"=="--check" goto check
if /i "%~1"=="--check-gl" goto check
echo Starting Monster Clash. WASD: move, 1/2/3: actions, P: pause.
"x64\Debug\OpenJL.exe"
if errorlevel 1 goto failed
goto success

:check
call "%VSROOT%\VC\Auxiliary\Build\vcvars64.bat"
if errorlevel 1 goto failed
cl /nologo /EHsc /std:c++20 Tests\BattleTests.cpp /Fo:x64\BattleTests.obj /Fe:x64\BattleTests.exe
if errorlevel 1 goto failed
"x64\BattleTests.exe"
if errorlevel 1 goto failed
cl /nologo /EHsc /std:c++20 Tests\SpawnLayoutTests.cpp /Fo:x64\SpawnLayoutTests.obj /Fe:x64\SpawnLayoutTests.exe
if errorlevel 1 goto failed
"x64\SpawnLayoutTests.exe"
if errorlevel 1 goto failed
cl /nologo /EHsc /std:c++20 Tests\HudStateTests.cpp /Fo:x64\HudStateTests.obj /Fe:x64\HudStateTests.exe
if errorlevel 1 goto failed
"x64\HudStateTests.exe"
if errorlevel 1 goto failed
cl /nologo /EHsc /std:c++20 /I Libraries\include Tests\EnemyNodeTests.cpp Game\EnemyNode.cpp Modules\MeshNode.cpp glad.c /Fo:x64\ /Fe:x64\EnemyNodeTests.exe /link /LIBPATH:Libraries\lib opengl32.lib
if errorlevel 1 goto failed
"x64\EnemyNodeTests.exe"
if errorlevel 1 goto failed
if /i not "%~1"=="--check-gl" goto checksPassed
copy /Y freeglutd.dll x64\freeglut.dll >nul
if errorlevel 1 goto failed
cl /nologo /EHsc /std:c++20 /I Libraries\include Tests\DissolveShaderTests.cpp glad.c /Fo:x64\ /Fe:x64\DissolveShaderTests.exe /link /LIBPATH:Libraries\lib opengl32.lib
if errorlevel 1 goto failed
"x64\DissolveShaderTests.exe"
if errorlevel 1 goto failed
:checksPassed
echo Build and regression checks passed.

:success
popd
exit /b 0

:failed
echo Failed. Read the error above. If OpenJL.exe is locked, close the running game and retry.
popd
exit /b 1
