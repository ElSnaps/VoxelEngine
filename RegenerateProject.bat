@echo off
set ProjectName=VoxelEngine

echo.
echo Clearing Binaries

if exist Binaries\ (
    rmdir /s /q "Binaries"
    echo Deleted Directory - %~dp0Binaries
) else (
    echo [No Clear Required] - %~dp0Binaries
)

:: Delete Directories for Code Workspaces
if exist .vs/ (
    rmdir /s /q ".vs"
    echo Deleted Directory - %~dp0.vs
)
if exist .vscode (
    rmdir /s /q ".vscode"
    echo Deleted Directory - %~dp0.vscode
)
if exist .idea (
    rmdir /s /q ".idea"
    echo Deleted Directory - %~dp0.idea
)

:: Delete Files for Code Workspaces, .sln ect
if exist %projectName%.xcworkspace (
    del /s /f /q "%projectName%.xcworkspace"
)
if exist %projectName%.sln (
    del /s /f /q "%projectName%.sln"
)
if exist %projectName%_Win64.sln (
    del /s /f /q "%projectName%_Win64.sln"
)
if exist %projectName%.code-workspace (
    del /s /f /q "%projectName%.code-workspace"
)

echo.
echo Regenerating Project, Please Wait.
echo.

:: Regenerate Project
cmake -G "Visual Studio 17 2022" -S ./ -B ./Binaries -DCMAKE_BUILD_TYPE=Release