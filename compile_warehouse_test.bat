@echo off
echo Compiling Warehouse Device State Test...

:: Clean previous build
if exist "test\WarehouseDeviceStateTest.exe" del "test\WarehouseDeviceStateTest.exe"

:: Compile the test
g++ -std=c++17 -Wall -g ^
    -I"include" ^
    -I"G:/SFML/SFML-2.6.2/include" ^
    -DSFML_STATIC ^
    src/test/WarehouseDeviceStateTest.cpp ^
    src/gui/WarehouseRenderer.cpp ^
    src/gui/TrackRenderer.cpp ^
    -o test/WarehouseDeviceStateTest.exe ^
    -L"G:/SFML/SFML-2.6.2/lib" ^
    -lsfml-graphics -lsfml-window -lsfml-system ^
    -lopengl32 -lwinmm -lgdi32

if %errorlevel% neq 0 (
    echo Compilation failed!
    pause
    exit /b 1
)

echo Compilation successful!
echo Running Warehouse Device State Test...
echo.

:: Run the test
cd test
WarehouseDeviceStateTest.exe
cd ..

echo.
echo Test completed.
pause
