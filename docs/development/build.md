# 构建说明

## 环境要求

### 必需工具

- Visual Studio 2022 或更高版本
- CMake 3.20 或更高版本
- Git
- PowerShell 7.0 或更高版本

### 依赖库

- SFML 2.6.0
- vcpkg（用于管理依赖）

## 构建步骤

### 1. 获取源代码

```bash
git clone https://github.com/yourusername/Warehouse-sch.git
cd Warehouse-sch
```

### 2. 安装依赖

使用 vcpkg 安装 SFML：

```bash
./vcpkg/vcpkg install sfml:x64-windows
```

### 3. 配置项目

使用 CMake 配置项目：

```bash
mkdir build
cd build
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake
```

### 4. 构建项目

#### 使用 Visual Studio

1. 打开生成的解决方案文件
2. 选择 Release 配置
3. 构建解决方案

#### 使用命令行

```bash
cmake --build . --config Release
```

### 5. 运行测试

```bash
ctest -C Release
```

## 常见问题

### 构建失败

#### CMake 配置错误

- 检查 CMake 版本是否满足要求
- 确认 vcpkg 路径正确
- 检查环境变量设置

#### 编译错误

- 检查 Visual Studio 版本
- 确认所有依赖库已正确安装
- 查看编译日志获取详细错误信息

### 运行时错误

#### 缺少 DLL

- 确保 Visual C++ Redistributable 已安装
- 检查 SFML DLL 是否在系统路径中

#### 资源文件缺失

- 确认资源文件已正确复制到输出目录
- 检查资源文件路径配置

## 发布构建

### 1. 准备发布版本

```bash
# 清理构建目录
rm -rf build/*
# 重新配置
cmake .. -DCMAKE_TOOLCHAIN_FILE=../vcpkg/scripts/buildsystems/vcpkg.cmake -DCMAKE_BUILD_TYPE=Release
# 构建
cmake --build . --config Release
```

### 2. 打包发布文件

```bash
# 创建发布目录
mkdir release
# 复制可执行文件
cp build/Release/WarehouseScheduler.exe release/
# 复制依赖DLL
cp vcpkg/installed/x64-windows/bin/*.dll release/
# 复制资源文件
cp -r assets release/
# 复制文档
cp -r docs release/
```

### 3. 创建安装程序

使用 NSIS 或其他安装程序制作工具创建安装程序。

## 持续集成

### GitHub Actions

项目使用 GitHub Actions 进行持续集成，配置文件位于`.github/workflows/`目录。

### 自动化测试

- 单元测试：使用 Google Test 框架
- 集成测试：使用自定义测试框架
- 性能测试：使用自定义基准测试工具

## 性能优化

### 编译优化

- 使用 Release 配置
- 启用链接时优化
- 使用多线程编译

### 运行时优化

- 使用性能分析工具
- 优化内存使用
- 减少不必要的计算

## 调试

### 调试配置

1. 在 Visual Studio 中：

   - 选择 Debug 配置
   - 设置断点
   - 使用调试工具

2. 使用日志：
   - 启用详细日志
   - 检查日志文件
   - 分析错误信息

### 性能分析

- 使用 Visual Studio 性能分析器
- 使用第三方性能分析工具
- 分析内存使用情况
