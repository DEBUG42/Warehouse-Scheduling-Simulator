# 开发指南

## 开发环境设置

### 必需工具

- Visual Studio 2022 或更高版本
- CMake 3.20 或更高版本
- Git
- PowerShell 7.0 或更高版本

### 依赖库

- SFML 2.6.0
- vcpkg（用于管理依赖）

## 构建步骤

1. 克隆仓库

```bash
git clone https://github.com/yourusername/Warehouse-sch.git
cd Warehouse-sch
```

2. 安装依赖

```bash
./vcpkg/vcpkg install sfml:x64-windows
```

3. 构建项目

```bash
./build/build.ps1
```

## 开发规范

### 代码风格

- 使用大驼峰命名法（PascalCase）命名类
- 使用小驼峰命名法（camelCase）命名函数和变量
- 使用下划线命名法（snake_case）命名常量

### 文件组织

- 源文件(.cpp)放在 src/ 目录
- 头文件(.hpp)放在 include/ 目录
- 资源文件放在 assets/ 目录

### 提交规范

- 提交信息格式：`类型: 简短描述`
- 类型包括：feat, fix, docs, style, refactor, test, chore

## 调试指南

### 常见问题

1. 构建失败

   - 检查 CMake 配置
   - 确认依赖库安装正确
   - 查看构建日志

2. 运行时错误
   - 检查资源文件路径
   - 确认 SFML 库加载正确
   - 查看错误日志

### 调试工具

- Visual Studio 调试器
- SFML 调试输出
- 日志系统

## 测试

### 单元测试

- 使用 Google Test 框架
- 测试文件放在 tests/ 目录
- 运行测试：`./build/run_tests.ps1`

### 集成测试

- 测试 GUI 功能
- 测试核心功能
- 测试性能

## 性能优化

### 渲染优化

- 使用批处理渲染
- 优化纹理加载
- 减少状态切换

### 内存优化

- 使用智能指针
- 避免不必要的拷贝
- 及时释放资源

## 发布流程

1. 版本号更新
2. 更新文档
3. 运行测试
4. 构建发布版本
5. 创建发布标签
6. 上传发布包
