# 物流仓库仿真系统构建脚本
# 作者：GitHub Copilot
# 日期：2025-05-19

# 检查CMake是否安装
if (-not (Get-Command cmake -ErrorAction SilentlyContinue)) {
    Write-Host "错误：未找到CMake。请安装CMake并确保它在PATH中。" -ForegroundColor Red
    exit 1
}

# 创建构建目录
$buildDir = "build"
if (-not (Test-Path $buildDir)) {
    New-Item -ItemType Directory -Path $buildDir | Out-Null
    Write-Host "创建构建目录: $buildDir" -ForegroundColor Green
}

# 进入构建目录
Push-Location $buildDir

try {
    # 检测可用生成器
    $generator = ""
    
    # 尝试使用Visual Studio生成器
    if (Get-Command "vswhere" -ErrorAction SilentlyContinue) {
        $vsPath = & vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        if ($vsPath) {
            $generator = "-G `"Visual Studio 17 2022`" -A x64"
        }
    }
    
    # 如果找不到VS，尝试使用Ninja
    if (-not $generator -and (Get-Command "ninja" -ErrorAction SilentlyContinue)) {
        $generator = "-G `"Ninja`""
    }
    
    # 如果找不到Ninja，使用默认生成器
    if (-not $generator) {
        $generator = ""
        Write-Host "未找到Visual Studio或Ninja，将使用默认生成器" -ForegroundColor Yellow
    }    # 运行CMake配置
    Write-Host "正在配置项目..." -ForegroundColor Cyan
    Invoke-Expression "cmake $generator .."
    
    if ($LASTEXITCODE -ne 0) {
        # 如果使用特定生成器失败，尝试使用默认生成器
        if ($generator) {
            Write-Host "使用指定生成器失败，尝试默认生成器..." -ForegroundColor Yellow
            cmake ..
            if ($LASTEXITCODE -ne 0) {
                Write-Host "CMake配置失败" -ForegroundColor Red
                exit $LASTEXITCODE
            }
        } else {
            Write-Host "CMake配置失败" -ForegroundColor Red
            exit $LASTEXITCODE
        }
    }
    
    # 构建项目
    Write-Host "正在构建项目..." -ForegroundColor Cyan
    cmake --build . --config Release
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "构建失败" -ForegroundColor Red
        exit $LASTEXITCODE
    }
    
    # 复制资源文件
    Write-Host "正在复制资源文件..." -ForegroundColor Cyan
    if (-not (Test-Path "bin\Release\resources")) {
        New-Item -ItemType Directory -Path "bin\Release\resources" -Force | Out-Null
    }
    
    if (Test-Path "..\resources") {
        Copy-Item -Path "..\resources\*" -Destination "bin\Release\resources" -Recurse -Force
        Write-Host "资源文件已复制" -ForegroundColor Green
    } else {
        Write-Host "警告：未找到资源目录" -ForegroundColor Yellow
    }
    
    # 显示成功信息
    Write-Host "构建成功!" -ForegroundColor Green
    Write-Host "可执行文件位置: $buildDir\bin\Release\WarehouseSimulation.exe" -ForegroundColor Green
    
    # 询问是否运行程序
    $runApp = Read-Host "是否立即运行程序？(Y/N)"
    if ($runApp -eq "Y" -or $runApp -eq "y") {
        Write-Host "正在启动程序..." -ForegroundColor Cyan
        Start-Process "bin\Release\WarehouseSimulation.exe"
    }
    
} finally {
    # 无论成功与否，恢复原始目录
    Pop-Location
}
