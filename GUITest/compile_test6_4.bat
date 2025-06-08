@echo off
echo Compiling test6_4.cpp...

set SFML_PATH=G:\SFML\SFML-2.6.2
set INCLUDE_PATH=-I%SFML_PATH%\include -Isrc -Iinclude
set LIB_PATH=-L%SFML_PATH%\lib
set LIBS=-lsfml-graphics -lsfml-window -lsfml-system

g++ %INCLUDE_PATH% %LIB_PATH% ^
    test\test6_4.cpp ^
    src\gui\Toolbar.cpp ^
    src\gui\StatusPanel.cpp ^
    src\gui\VehicleInfoPanel.cpp ^
    src\gui\UIControls.cpp ^
    src\gui\SimulationView.cpp ^
    src\gui\VehicleRenderer.cpp ^
    src\gui\TrackRenderer.cpp ^
    src\gui\WarehouseRenderer.cpp ^
    src\gui\ObjectInspector.cpp ^
    src\gui\TaskListView.cpp ^
    src\Physics\AssignTaskToVehicles.cpp ^
    src\Physics\Logger.cpp ^
    src\Physics\Scheduler.cpp ^
    src\Physics\TaskManager.cpp ^
    src\Physics\DeviceManager.cpp ^
    src\Physics\EventQueue.cpp ^
    %LIBS% ^
    -o bin\test6_4.exe

if %ERRORLEVEL% == 0 (
    echo Compilation successful! Running test6_4.exe...
    bin\test6_4.exe
) else (
    echo Compilation failed!
    pause
)
