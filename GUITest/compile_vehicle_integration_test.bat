@echo off
echo ========================================
echo Building VehicleInfoPanel Integration Test
echo ========================================

set SFML_PATH=G:\SFML\SFML-2.6.2
set MINGW_PATH=G:\mingw64
set PROJECT_PATH=d:\CodeMaster\GitHub\C++SFML\Warehouse-sch

echo Setting up build environment...
set PATH=%MINGW_PATH%\bin;%PATH%

echo Cleaning previous build...
if exist "%PROJECT_PATH%\test\VehicleInfoPanelIntegrationTest.exe" (
    del "%PROJECT_PATH%\test\VehicleInfoPanelIntegrationTest.exe"
)

echo Compiling VehicleInfoPanel Integration Test...
g++ -std=c++17 ^
    -I"%PROJECT_PATH%\include" ^
    -I"%PROJECT_PATH%\src" ^
    -I"%SFML_PATH%\include" ^
    -DSFML_STATIC ^
    -O2 ^
    -Wall ^
    -Wextra ^
    "%PROJECT_PATH%\test\VehicleInfoPanelIntegrationTest.cpp" ^
    "%PROJECT_PATH%\src\gui\MainWindow.cpp" ^
    "%PROJECT_PATH%\src\gui\Toolbar.cpp" ^
    "%PROJECT_PATH%\src\gui\StatusPanel.cpp" ^
    "%PROJECT_PATH%\src\gui\VehicleInfoPanel.cpp" ^
    "%PROJECT_PATH%\src\gui\SimulationView.cpp" ^
    "%PROJECT_PATH%\src\gui\ObjectInspector.cpp" ^
    "%PROJECT_PATH%\src\gui\TaskListView.cpp" ^
    "%PROJECT_PATH%\src\gui\UIControls.cpp" ^
    "%PROJECT_PATH%\src\gui\VehicleRenderer.cpp" ^
    "%PROJECT_PATH%\src\gui\TrackRenderer.cpp" ^
    "%PROJECT_PATH%\src\gui\WarehouseRenderer.cpp" ^
    "%PROJECT_PATH%\src\gui\MockSimulationInterface.cpp" ^
    -L"%SFML_PATH%\lib" ^
    -lsfml-graphics-s ^
    -lsfml-window-s ^
    -lsfml-system-s ^
    -lopengl32 ^
    -lfreetype ^
    -lwinmm ^
    -lgdi32 ^
    -static-libgcc ^
    -static-libstdc++ ^
    -o "%PROJECT_PATH%\test\VehicleInfoPanelIntegrationTest.exe"

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo BUILD SUCCESSFUL!
    echo ========================================
    echo Executable created: %PROJECT_PATH%\test\VehicleInfoPanelIntegrationTest.exe
    echo.
    
    echo Running integration test...
    echo ========================================
    cd /d "%PROJECT_PATH%"
    "%PROJECT_PATH%\test\VehicleInfoPanelIntegrationTest.exe"
    
    echo.
    echo Integration test completed.
    echo Check the console output above for test results.
) else (
    echo.
    echo ========================================
    echo BUILD FAILED!
    echo ========================================
    echo Please check the error messages above.
    echo Common issues:
    echo - Missing SFML libraries
    echo - Missing source files
    echo - Header file dependencies
    echo.
)

echo.
echo Press any key to continue...
pause >nul
