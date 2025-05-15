@echo off
REM ============================================
REM 仓储穿梭车仿真系统GUI编译脚本
REM ============================================

echo 开始编译仿真系统...

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

REM 源文件
set SRC_GUI=GUITest_EN.cpp
REM set SRC_CORE=..\Core\SimulationEngine.cpp ..\Core\Vehicle.cpp ..\Core\Device.cpp
REM set SRC_PHYSICS=..\Physics\MotionController.cpp

REM 编译命令
echo 编译中...
%COMPILER% %STD% %DEBUG% %WARNINGS% %INCLUDE_PATH% %SRC_GUI% -o GUITest.exe %LIB_PATH% %SFML_LIBS%

REM 复制SFML DLL到可执行文件目录
echo 复制SFML DLL...
REM copy "%SFML_PATH%\bin\*.dll" .

IF %ERRORLEVEL% NEQ 0 (
    echo 编译失败！请检查错误信息。
    exit /b %ERRORLEVEL%
)

echo 编译成功！
echo 运行仿真系统...
GUITest.exe
