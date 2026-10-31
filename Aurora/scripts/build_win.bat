@echo off
:: ============================================================================
:: Aurora build script (Windows)
:: Builds the AzerothCore-based server with the Aurora custom layer.
:: ============================================================================

setlocal

set ROOT=%~dp0..
set BUILD=build
set CONFIG=RelWithDebInfo
set GEN="Visual Studio 17 2022"
set MSG_COLOR=0A

::_colors====================================================================
:main
cls
echo.
echo   Aurora build script
echo   Target: %GEN%
echo.

:: --- check tools ---------------------------------------------------------
where cmake >nul 2>&1
if errorlevel 1 (
    echo [ERROR] cmake not found in PATH. Install CMake (or add to PATH).
    pause
    exit /b 1
)

where msbuild >nul 2>&1
if errorlevel 1 (
    echo [ERROR] MSBuild not found. Install Visual Studio with C++ tools.
    pause
    exit /b 1
)

:: --- configure -----------------------------------------------------------
set CONFIGURE_ARGS=-S "%ROOT%" -B "%ROOT%\%BUILD%" -G %GEN% -DCMAKE_BUILD_TYPE=%CONFIG%
echo [1/3] Configuring...
cmake %CONFIGURE_ARGS%
if errorlevel 1 (
    echo [ERROR] Configure failed.
    pause
    exit /b 1
)

:: --- build ---------------------------------------------------------------
echo [2/3] Building...
cmake --build "%ROOT%\%BUILD%" --config %CONFIG%
if errorlevel 1 (
    echo [ERROR] Build failed.
    pause
    exit /b 1
)

:: --- summary -------------------------------------------------------------
echo.
echo [3/3] Build finished.
echo.
echo Binaries:
echo   - WorldServer: %ROOT%\%BUILD%\bin\Release\worldserver.exe
echo   - AuthServer:  %ROOT%\%BUILD%\bin\Release\authserver.exe
echo.
echo Run:
echo   scripts\run_auth.bat
echo   scripts\run_world.bat
echo.

pause
