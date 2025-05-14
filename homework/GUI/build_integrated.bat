@echo off
REM ============================================
REM 仓储穿梭车仿真系统GUI整合测试编译脚本
REM ============================================

echo 开始编译仿真系统整合测试...

REM 设置SFML路径（根据实际情况修改）
set SFML_PATH=G:/SFML/SFML-2.6.2
set INCLUDE_PATH=-I"%SFML_PATH%/include"
set LIB_PATH=-L"%SFML_PATH%/lib"

REM 编译选项
set COMPILER=g++
set STD=-std=c++17
set DEBUG=-g
set WARNINGS=-Wall -Wextra
set SFML_LIBS=-lsfml-graphics -lsfml-window -lsfml-system

REM 源文件 - 所有GUI类文件
set SRC_GUI=IntegratedGUITest.cpp MainWindow.cpp SimulationView.cpp TrackRenderer.cpp DeviceRenderer.cpp VehicleRenderer.cpp StatusPanel.cpp Toolbar.cpp TaskList.cpp UIControls.cpp

REM 编译命令
echo 编译中...
%COMPILER% %STD% %DEBUG% %WARNINGS% %INCLUDE_PATH% %SRC_GUI% -o IntegratedGUITest.exe %LIB_PATH% %SFML_LIBS%

IF %ERRORLEVEL% NEQ 0 (
    echo 编译失败！请检查错误信息。
    exit /b %ERRORLEVEL%
)

echo 编译成功！
echo 运行整合测试...
IntegratedGUITest.exe
