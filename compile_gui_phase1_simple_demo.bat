@echo off
echo Compiling GUI_Phase1_Simple_Demo.cpp...

set SFML_PATH=G:\SFML\SFML-2.6.2
set INCLUDE_PATH=-I"%SFML_PATH%\include" -I"src" -I"include"
set LIB_PATH=-L"%SFML_PATH%\lib"
set LIBS=-lsfml-graphics -lsfml-window -lsfml-system

echo Include paths: %INCLUDE_PATH%
echo Library path: %LIB_PATH%
echo Libraries: %LIBS%
echo.

g++ %INCLUDE_PATH% %LIB_PATH% ^
    test\GUI_Phase1_Simple_Demo.cpp ^
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
    %LIBS% ^
    -o test\GUI_Phase1_Simple_Demo.exe

if %ERRORLEVEL% == 0 (
    echo Compilation successful! Running the executable...
    test\GUI_Phase1_Simple_Demo.exe
) else (
    echo Compilation failed!
    pause
)
