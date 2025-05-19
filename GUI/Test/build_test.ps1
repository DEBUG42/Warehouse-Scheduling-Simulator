# 物流仓库仿真系统测试构建脚本
# 用于编译和运行GUI测试程序

# 设置环境变量
$ErrorActionPreference = "Stop"
$sfmlPath = "G:/SFML/SFML-2.6.2"  # 请根据实际SFML路径修改

# 如果SFML路径不存在，提示用户修改
if (-not (Test-Path $sfmlPath)) {
    Write-Host "错误：SFML路径不存在，请在脚本中修改`$sfmlPath变量为正确的SFML安装路径" -ForegroundColor Red
    exit 1
}

# 创建输出目录
$outputDir = ".\Test\build"
if (-not (Test-Path $outputDir)) {
    New-Item -ItemType Directory -Path $outputDir | Out-Null
    Write-Host "已创建输出目录: $outputDir" -ForegroundColor Green
}

# 编译测试程序
function Build-Test {
    param (
        [string]$sourceFile,
        [string]$outputFile
    )
    
    Write-Host "正在编译 $sourceFile..." -ForegroundColor Cyan
    
    try {
        g++ -o $outputFile $sourceFile `
            -I"$sfmlPath/include" `
            -DSFML_STATIC `
            -L"$sfmlPath/lib" `
            -lsfml-graphics -lsfml-window -lsfml-system `
            -std=c++17
            
        if ($LASTEXITCODE -eq 0) {
            Write-Host "编译成功: $outputFile" -ForegroundColor Green
            return $true
        } else {
            Write-Host "编译失败，返回代码: $LASTEXITCODE" -ForegroundColor Red
            return $false
        }
    }
    catch {
        Write-Host "编译过程中发生错误: $_" -ForegroundColor Red
        return $false
    }
}

# 运行测试程序
function Run-Test {
    param (
        [string]$execFile
    )
    
    Write-Host "正在运行 $execFile..." -ForegroundColor Cyan
    try {
        & $execFile
        if ($LASTEXITCODE -eq 0) {
            Write-Host "测试程序正常退出" -ForegroundColor Green
        } else {
            Write-Host "测试程序异常退出，返回代码: $LASTEXITCODE" -ForegroundColor Yellow
        }
    }
    catch {
        Write-Host "运行测试程序时发生错误: $_" -ForegroundColor Red
    }
}

# 检查g++是否可用
try {
    $gppVersion = g++ --version
    Write-Host "使用编译器: $($gppVersion[0])" -ForegroundColor Blue
}
catch {
    Write-Host "错误：找不到g++编译器，请确保已安装并添加到PATH环境变量中" -ForegroundColor Red
    exit 1
}

# 构建参数
$sourceFile = ".\Test\GUITest_Fixed.cpp"
$outputFile = "$outputDir\GUITest_Fixed.exe"

# 检查源文件是否存在
if (-not (Test-Path $sourceFile)) {
    Write-Host "错误：源文件不存在: $sourceFile" -ForegroundColor Red
    exit 1
}

# 构建并运行测试程序
$buildSuccess = Build-Test -sourceFile $sourceFile -outputFile $outputFile
if ($buildSuccess) {
    $runTest = Read-Host "是否要立即运行测试程序? (Y/N)"
    if ($runTest -eq "Y" -or $runTest -eq "y") {
        Run-Test -execFile $outputFile
    }
}

# 构建完成
Write-Host "构建脚本执行完成" -ForegroundColor Blue
