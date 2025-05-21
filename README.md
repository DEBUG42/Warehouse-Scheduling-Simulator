# 仓库调度系统

基于 SFML 的仓库调度仿真系统，用于模拟和可视化仓库设备的运行状态和任务调度过程。

## 功能特点

- 3D 可视化仓库轨道和设备系统
- 实时显示设备状态和任务执行
- 交互式视角控制
- 任务调度和管理
- 数据统计和导出
- 可扩展的插件系统

## 系统要求

### 硬件要求

- CPU: Intel Core i5 或更高
- 内存: 8GB 或更高
- 显卡: 支持 OpenGL 3.3 或更高
- 硬盘空间: 至少 1GB 可用空间

### 软件要求

- Windows 10 或更高版本
- Visual C++ Redistributable 2019 或更高版本

## 快速开始

### 安装

1. 下载最新发布版本
2. 解压到任意目录
3. 运行 `WarehouseScheduler.exe`

### 从源码构建

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

## 使用指南

### 基本操作

- 查看设备状态：在左侧设备列表中选择设备
- 添加新任务：点击"新建任务"按钮
- 监控任务：在右侧任务面板查看任务列表
- 调整视图：使用鼠标进行平移和缩放

### 高级功能

- 批量任务管理
- 数据导出
- 系统设置

## 开发指南

### 环境设置

- Visual Studio 2022
- CMake 3.20+
- SFML 2.6.0
- vcpkg

### 构建步骤

1. 安装依赖
2. 配置项目
3. 构建项目
4. 运行测试

## 文档

- [用户手册](docs/user/manual.md)
- [开发指南](docs/development/guide.md)
- [API 文档](docs/api/index.html)
- [常见问题](docs/user/faq.md)

## 贡献

1. Fork 项目
2. 创建功能分支
3. 提交更改
4. 推送到分支
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证 - 详见[LICENSE](LICENSE)文件

## 联系方式

- 邮箱：support@example.com
- 问题反馈：https://github.com/yourusername/Warehouse-sch/issues
- 文档更新：https://github.com/yourusername/Warehouse-sch/wiki

## 致谢

- [SFML](https://www.sfml-dev.org/)
- [vcpkg](https://github.com/microsoft/vcpkg)
- [CMake](https://cmake.org/)
- 所有贡献者
