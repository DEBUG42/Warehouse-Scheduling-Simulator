# PowerShell构建脚本
$ErrorActionPreference = "Stop"

# 创建构建目录
$buildDir = "build"
if (!(Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
    Write-Host "创建构建目录: $buildDir" -ForegroundColor Green
}

# 进入构建目录
Push-Location $buildDir

try {
    # 检查是否安装了CMake
    if (!(Get-Command cmake -ErrorAction SilentlyContinue)) {
        Write-Host "错误: 未找到CMake。请安装CMake并确保其在系统PATH中。" -ForegroundColor Red
        exit 1
    }
    
    # 检测编译器
    $compiler = "Visual Studio 17 2022"
    
    # 如果设置了USE_MINGW环境变量，使用MinGW
    if ($env:USE_MINGW -eq "1") {
        $compiler = "MinGW Makefiles"
        Write-Host "使用MinGW编译器" -ForegroundColor Yellow
    }
    
    # 配置CMake项目
    Write-Host "配置CMake项目..." -ForegroundColor Cyan
    cmake -G "$compiler" ..
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "CMake配置失败！" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # 构建项目
    Write-Host "构建项目..." -ForegroundColor Cyan
    cmake --build . --config Release
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "构建失败！" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    Write-Host "构建成功！" -ForegroundColor Green
    Write-Host "可执行文件位于: $buildDir\bin\Release\WarehouseSimulation.exe" -ForegroundColor Green
}
finally {
    # 无论如何都恢复原来的目录
    Pop-Location
}
