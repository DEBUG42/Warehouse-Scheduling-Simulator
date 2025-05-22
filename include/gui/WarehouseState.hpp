#pragma once
#include <SFML/Graphics.hpp>
#include "DeviceState.hpp"

struct WarehouseState
{
    int id;                   // 仓库ID
    gui::DeviceType type;     // 仓库类型
    gui::DeviceStatus status; // 当前状态
    float processingProgress; // 处理进度(0-1)
    int queuedTaskCount;      // 等待任务数
    sf::Vector2f position;    // 仓库位置
};