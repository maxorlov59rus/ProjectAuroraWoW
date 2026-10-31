@echo off
:: ============================================================================
:: Cleanup duplicates for the Aurora project scaffolding
// ============================================================================
setlocal

set "DIR=%~dp0src\Aurora"

rem Class headers already have an implementation (.cpp) that we replace below.
rem Here we remove the duplicate/declaration-only .cpp files that must not be
:: compiled twice:
del /F /Q "%DIR%\AuroraGameMode.cpp"
del /F /Q "%DIR%\AuroraSkills.cpp"
del /F /Q "%DIR%\AuroraSkins.cpp"
del /F /Q "%DIR%\AuroraInterface.cpp"
del /F /Q "%DIR%\AuroraLogging.cpp"
del /F /Q "%DIR%\AuroraGameModesModule.cpp"
del /F /Q "%DIR%\AuroraSkillsModule.cpp"
del /F /Q "%DIR%\AuroraSkinsModule.cpp"

echo Cleanup done.
pause
