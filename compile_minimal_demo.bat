@echo off
cd "d:\CodeMaster\GitHub\C++SFML\Warehouse-sch"
g++ -std=c++17 -g -I"include" -I"src" -I"G:/SFML/SFML-2.6.2/include" test/GUI_Phase1_Minimal_Demo.cpp src/gui/Toolbar.cpp src/gui/UIControls.cpp src/gui/SimulationView.cpp src/gui/TrackRenderer.cpp src/gui/VehicleRenderer.cpp src/gui/WarehouseRenderer.cpp -o test/GUI_Phase1_Minimal_Demo.exe -L"G:/SFML/SFML-2.6.2/lib" -lsfml-graphics -lsfml-window -lsfml-system -Wl,--subsystem,console

if %errorlevel% equ 0 (
    echo Compilation successful!
    test\GUI_Phase1_Minimal_Demo.exe
) else (
    echo Compilation failed!
    pause
)
