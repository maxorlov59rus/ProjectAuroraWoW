@echo off
:: ============================================================================
:: Aurora Windows Build Test Script
:: This script verifies the build process on Windows
:: ============================================================================

setlocal

set ROOT=%~dp0..
set BUILD=build
set CONFIG=RelWithDebInfo
set GEN="Visual Studio 17 2022"

echo ============================================
echo Aurora Windows Build Test
echo ============================================
echo.

:: Check tools
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] CMake not found in PATH
    exit /b 1
)

where msbuild >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSBuild not found in PATH
    exit /b 1
)

echo [1/4] Configuring CMake...
cmake -S "%ROOT%" -B "%ROOT%\%BUILD%" -G %GEN% -DCMAKE_BUILD_TYPE=%CONFIG%
if errorlevel 1 (
    echo [ERROR] Configure failed
    exit /b 1
)

echo [2/4] Building server...
cmake --build "%ROOT%\%BUILD%" --config %CONFIG% --parallel 4
if errorlevel 1 (
    echo [ERROR] Build failed
    exit /b 1
)

echo [3/4] Verifying binaries...
if not exist "%ROOT%\%BUILD%\bin\%CONFIG%\worldserver.exe" (
    echo [ERROR] worldserver.exe not found
    exit /b 1
)
if not exist "%ROOT%\%BUILD%\bin\%CONFIG%\authserver.exe" (
    echo [ERROR] authserver.exe not found
    exit /b 1
)

echo [4/4] Build test PASSED!
echo.
echo Binaries:
echo   - WorldServer: %ROOT%\%BUILD%\bin\%CONFIG%\worldserver.exe
echo   - AuthServer:  %ROOT%\%BUILD%\bin\%CONFIG%\authserver.exe
echo.
echo To run:
echo   scripts\run_auth.bat
echo   scripts\run_world.bat

exit /b 0