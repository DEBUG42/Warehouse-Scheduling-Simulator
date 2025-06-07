@echo off
chcp 65001 > nul

:: Configuration for testtask1.cpp compilation
set "COMPILER=g++"
set "SFML_INC=D:\SFML\include"
set "SFML_LIB=D:\SFML\lib"
set "OUTPUT=bin\testtask1.exe"

:: Verify SFML paths
if not exist "%SFML_INC%" (
    echo Error: SFML include path not found at %SFML_INC%
    pause
    exit /b 1
)

if not exist "%SFML_LIB%" (
    echo Error: SFML lib path not found at %SFML_LIB%
    pause
    exit /b 1
)

:: Create output directory
if not exist "bin" mkdir "bin"

:: Compile command
echo Compiling testtask1.cpp...
%COMPILER% -std=c++17 -Wall -DSFML_STATIC ^
    -I"%SFML_INC%" ^
    -I"include" ^
    -I"src" ^
    -I"include/gui" ^
    -I"src/Core" ^
    -I".." ^
    "test/testtask1.cpp" ^
    "src/gui/SimulationView.cpp" ^
    "src/gui/Toolbar.cpp" ^
    "src/gui/StatusPanel.cpp" ^
    "src/gui/VehicleInfoPanel.cpp" ^
    "src/gui/UIControls.cpp" ^
    "src/Physics/Logger.cpp" ^
    "src/Physics/Scheduler.cpp" ^
    "src/Physics/DeviceManager.cpp" ^
    "src/Physics/EventQueue.cpp" ^
    "src/Physics/TaskManager.cpp" ^
    "src/Physics/AssignTaskToVehicles.cpp" ^
    "src/gui/TrackRenderer.cpp" ^
    "src/gui/VehicleRenderer.cpp" ^
    "src/gui/WarehouseRenderer.cpp" ^
    "src/gui/TaskListView.cpp" ^
    "src/gui/ObjectInspector.cpp" ^
    -L"%SFML_LIB%" ^
    -g -mwindows -Wl,--subsystem,windows -static ^
    -lsfml-graphics-s -lsfml-window-s -lsfml-system-s ^
    -lsfml-audio-s -lsfml-network-s ^
    -lopengl32 -lfreetype -lwinmm -lgdi32 -lopenal32 -lflac -lvorbisenc -lvorbisfile -lvorbis -logg ^
    -static-libgcc -static-libstdc++ ^
    -o "%OUTPUT%"

:: Check result
if %errorlevel% equ 0 (
    echo Compilation successful: %OUTPUT%
    echo Run with: %OUTPUT%
) else (
    echo Compilation failed with error: %errorlevel%
    pause
    exit /b %errorlevel%
)
