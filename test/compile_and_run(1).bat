@echo off
chcp 65001 > nul

:: Basic configuration
set "COMPILER=g++"
set "SFML_INC=D:\SFML\include"
set "SFML_LIB=D:\SFML\lib"
set "OUTPUT=bin\WarehouseSchedulingSimulator.exe"

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
echo Compiling Warehouse Scheduling Simulator...
%COMPILER% -std=c++17 -Wall ^
    -DTimescale=1.0 ^
    -I"%SFML_INC%" ^
    -I"Warehouse-Scheduling-Simulator" ^
    -I"Warehouse-Scheduling-Simulator/Core" ^
    -I"Warehouse-Scheduling-Simulator/Physics" ^
    "Warehouse-Scheduling-Simulator/main.cpp" ^
    "Warehouse-Scheduling-Simulator/Physics/EventQueue.cpp" ^
    "Warehouse-Scheduling-Simulator/Physics/Logger.cpp" ^
    "Warehouse-Scheduling-Simulator/Physics/Scheduler.cpp" ^
    "Warehouse-Scheduling-Simulator/Physics/AssignTaskToVehicles.cpp" ^
    "Warehouse-Scheduling-Simulator/Physics/DeviceManager.cpp" ^
    "Warehouse-Scheduling-Simulator/Physics/TaskManager.cpp" ^
    -L"%SFML_LIB%" ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -o "%OUTPUT%"

:: Check result
if %errorlevel% equ 0 (
    echo Compilation successful: %OUTPUT%
    "%OUTPUT%"
) else (
    echo Compilation failed with error: %errorlevel%
    pause
    exit /b %errorlevel%
)
