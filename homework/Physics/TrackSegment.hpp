#pragma once

// 轨道段信息结构体
struct TrackSegment
{
    enum class Type
    {
        Straight, // 直轨段
        Curve     // 弯轨段
    };

    Type type;           // 轨道类型
    float startPosition; // 起始位置
    float endPosition;   // 结束位置
    float length;        // 段长度

    // 用于弯轨的参数
    float radius;     // 弯道半径
    float centerX;    // 弯道中心X坐标
    float centerY;    // 弯道中心Y坐标
    float startAngle; // 起始角度
    float endAngle;   // 结束角度
};

// 货物信息结构
struct CargoInfo
{
    int materialId;     // 物料编号
    bool isEmpty;       // 是否为空载状态
    int sourceDeviceId; // 来源设备ID
    int targetDeviceId; // 目标设备ID
};
