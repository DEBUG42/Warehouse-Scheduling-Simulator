@echo off
chcp 65001 > nul

echo Testing VehicleInfoPanel Integration...

:: SFML configuration
set "COMPILER=g++"
set "SFML_INC=G:\SFML\SFML-2.6.2\include"
set "SFML_LIB=G:\SFML\SFML-2.6.2\lib"
set "OUTPUT=bin\test_vehicle_integration.exe"

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

:: Compile VehicleInfoPanel integration test
echo Compiling VehicleInfoPanel Integration Test...
%COMPILER% -std=c++17 -Wall ^
    -I"%SFML_INC%" ^
    -I"include" ^
    -I"src" ^
    "test/SimpleVehicleInfoTest.cpp" ^
    "src/gui/VehicleInfoPanel.cpp" ^
    -L"%SFML_LIB%" ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -o "%OUTPUT%"

:: Check result
if %errorlevel% equ 0 (
    echo Compilation successful: %OUTPUT%
    echo Running integration test...
    "%OUTPUT%"
) else (
    echo Compilation failed with error: %errorlevel%
    pause
    exit /b %errorlevel%
)
