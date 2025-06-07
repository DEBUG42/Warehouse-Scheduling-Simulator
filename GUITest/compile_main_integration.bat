@echo off
echo 编译前后端集成的主程序...

set SFML_DIR=G:\SFML\SFML-2.6.2
set TGUI_DIR=third_party\TGUI

g++ -Wall -Wextra -g ^
    -I"include" ^
    -I"include/gui" ^
    -I"src" ^
    -I"%SFML_DIR%\include" ^
    -I"%TGUI_DIR%\include" ^
    -DSFML_STATIC ^
    -DTGUI_STATIC ^
    src\gui\main.cpp ^
    src\gui\MainWindow.cpp ^
    src\gui\SimulationView.cpp ^
    src\gui\VehicleRenderer.cpp ^
    src\gui\Toolbar.cpp ^
    src\gui\TaskListView.cpp ^
    src\gui\RealBackendAdapter.cpp ^
    -o bin\MainIntegrationTest.exe ^
    -L"%SFML_DIR%\lib" ^
    -L"%TGUI_DIR%\lib" ^
    -lsfml-graphics-s ^
    -lsfml-window-s ^
    -lsfml-system-s ^
    -ltgui-s ^
    -lopengl32 ^
    -lgdi32 ^
    -lwinmm ^
    -lfreetype

if %ERRORLEVEL% EQU 0 (
    echo 编译成功！可执行文件位于: bin\MainIntegrationTest.exe
    echo.
    echo 运行程序...
    bin\MainIntegrationTest.exe
) else (
    echo 编译失败！
    pause
)
