#pragma once

namespace Config
{

    // 显示比例
    constexpr float MM_TO_PIXEL = 0.05f; // 1毫米对应0.05像素

    // 窗口配置
    constexpr int WINDOW_WIDTH = 1280;
    constexpr int WINDOW_HEIGHT = 720;
    constexpr char WINDOW_TITLE[] = "仓库调度系统";

    // 轨道配置
    constexpr float TRACK_WIDTH = 600.0f;         // 轨道宽度（毫米）
    constexpr float TRACK_LENGTH = 40000.0f;      // 直线轨道长度（毫米）
    constexpr float TRACK_CURVE_RADIUS = 2500.0f; // 弯道半径（毫米）

    // 车辆配置
    constexpr float VEHICLE_LENGTH = 2000.0f;    // 车辆长度（毫米）
    constexpr float VEHICLE_WIDTH = 800.0f;      // 车辆宽度（毫米）
    constexpr float VEHICLE_MAX_SPEED = 5000.0f; // 最大速度（毫米/秒）

    // 设备配置
    constexpr float DEVICE_SIZE = 1000.0f; // 设备尺寸（毫米）
    constexpr int MAX_DEVICES = 18;        // 最大设备数量

    // 颜色配置
    struct Colors
    {
        static constexpr unsigned int TRACK = 0x808080FF;          // 轨道颜色（灰色）
        static constexpr unsigned int VEHICLE_EMPTY = 0x0000FFFF;  // 空载车辆颜色（蓝色）
        static constexpr unsigned int VEHICLE_LOADED = 0xFFA500FF; // 载货车辆颜色（橙色）
        static constexpr unsigned int DEVICE_NORMAL = 0x00FF00FF;  // 正常设备颜色（绿色）
        static constexpr unsigned int DEVICE_BUSY = 0xFF0000FF;    // 忙碌设备颜色（红色）
        static constexpr unsigned int DEVICE_ERROR = 0x800080FF;   // 故障设备颜色（紫色）
    };

    // 更新配置
    constexpr float UPDATE_INTERVAL = 16.0f; // 更新间隔（毫秒）
    constexpr float SIMULATION_SPEED = 1.0f; // 仿真速度倍率

    // 调试配置
    constexpr bool SHOW_GRID = true;        // 是否显示网格
    constexpr bool SHOW_LABELS = true;      // 是否显示标签
    constexpr bool SHOW_DEBUG_INFO = false; // 是否显示调试信息

} // namespace Config