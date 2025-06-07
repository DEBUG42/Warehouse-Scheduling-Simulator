#include "gui/WarehouseRenderer.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#define _USE_MATH_DEFINES // For M_PI in MSVC
#include <cmath>          // For std::abs, M_PI if needed elsewhere

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "../Core/Device.hpp" // No Core:: prefix needed for types from here

/*
每一个仓库对应在弯道上的位置，
根据开发界面信息规定的将整个轨道左下角弯道与直道的交汇点作为原点，
以中心线作为一位的distance单位，
每个仓库接口的距离单位为毫米，
计算出每个仓库接口应该对应的位置

轨道参数:
- 直道长度 (L_straight): 40000mm
- 内弯道半径 (R_inner): 2500mm
- 轨道宽度 (W_track): 1200mm
- 中心线弯道半径 (R_center): R_inner + W_track/2 = 2500mm + 600mm = 3100mm
- PI: 3.14159265358979323846

仓库距离计算 (基于中心线):
- 底部直道仓库 (ID 13-18): 直接使用距离值。
- 上部弯道+直道仓库 (ID 1-12):
  - 距离 = L_straight (底部直道) + PI * R_center (右侧弯道中心线弧长) + L_upper_segment (上部直道段)
  - 例如 WH12 (用户提供公式): 40000 + 2500pi [实际应为 3100pi] + 3800 = 最终值 53520.9372261538mm
  - 注：用户提供的计算过程中的 "2500pi" 项，其最终结果值是基于中心线半径 3100mm 计算得出的。
  - 因此，s_deviceLayouts 中存储的 trackDistanceMm 均为中心线距离。

提供的中心线距离数据 (最终值):
ID 18: 8000.0f
ID 17: 11100.0f
ID 16: 14000.0f
ID 15: 26000.0f
ID 14: 29000.0f
ID 13: 32000.0f
ID 12: 53520.9372261538f
ID 11: 55920.9372261538f
ID 10: 59520.9372261538f
ID 9:  61920.9372261538f
ID 8:  65520.9372261538f
ID 7:  67920.9372261538f
ID 6:  71520.9372261538f
ID 5:  73920.9372261538f
ID 4:  77520.9372261538f
ID 3:  79920.9372261538f
ID 2:  83520.9372261538f
ID 1:  85920.9372261538f
*/
/*
        WarehouseInterface interface_element; // Renamed to avoid conflict with member name
        interface_element.id = layout.id;
        interface_element.type = layout.type;
        interface_element.positionCategory = layout.positionCategory;
        interface_element.widthMm = layout.visualWidthMm / 2.0f;
        interface_element.depthMm = layout.visualDepthMm / 2.0f; // Use half depth for correct positioning

*/
// Define the fixed layout based on comments and image from user
// All TrackDist MM are CENTERLINE distances.
// Add Core::DeviceType to the layout definition
const std::vector<WarehouseRenderer::PredefinedDeviceLayout> WarehouseRenderer::s_deviceLayouts = {
    // Bottom track devices (IDs 13-18) - Centerline Distances
    // Device ID, TrackDist MM (Centerline), GUI Type, GUI positionCategory, VisualWidth MM, VisualDepth MM, OffsetFromTrackEdge MM, CoreType
    {18, 8000.0f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1100, 2700, 200, DeviceType::WorkstationIn},
    {17, 11100.0f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1100, 2700, 200, DeviceType::WorkstationIn},
    {16, 14000.0f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1100, 2700, 200, DeviceType::WorkstationIn},
    {15, 26000.0f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1100, 2700, 200, DeviceType::WorkstationOut},
    {14, 29000.0f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1100, 2700, 200, DeviceType::WorkstationOut},
    {13, 32000.0f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1100, 2700, 200, DeviceType::WorkstationOut},

    // Top track devices (IDs 1-12) - Centerline Distances
    // Image: Odd IDs are INPUT_STATION (arrow in), Even IDs are OUTPUT_STATION (arrow out) for top row.
    {12, 53520.9372261538f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageOut},
    {11, 55920.9372261538f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageIn},
    {10, 59520.9372261538f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageOut},
    {9, 61920.9372261538f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageIn},
    {8, 65520.9372261538f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageOut},
    {7, 67920.9372261538f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageIn},
    {6, 71520.9372261538f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageOut},
    {5, 73920.9372261538f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageIn},
    {4, 77520.9372261538f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageOut},
    {3, 79920.9372261538f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageIn},
    {2, 83520.9372261538f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageOut},
    {1, 85920.9372261538f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1100, 2700, 200, DeviceType::StorageIn},
};

WarehouseRenderer::WarehouseRenderer()
{
    if (!m_font.loadFromFile("C:/Windows/Fonts/COOPBL.TTF"))
    {
        if (!m_font.loadFromFile("../../../assets/fonts/arial.ttf"))
        {
            std::cerr << "WarehouseRenderer - Font Arial.ttf not found. Interface labels may not display correctly." << std::endl;
        }
    }
}

void WarehouseRenderer::initialize(TrackRenderer &trackRenderer, const sf::Vector2f &worldOriginOffset, const std::string &iconBasePath)
{
    m_trackRendererRef = &trackRenderer;
    m_worldOriginOffsetPx = worldOriginOffset; // Store the world origin offset
    m_interfaces.clear();
    m_labels.clear();

    sf::Texture storageInTexture, storageOutTexture, workInTexture, workOutTexture, chargeTexture, defaultTexture;
    bool loadSuccess = true;
    auto loadTex = [&](sf::Texture &tex, const std::string &filename)
    {
        if (!tex.loadFromFile(iconBasePath + filename))
        {
            std::cerr << "WarehouseRenderer - Failed to load icon: " << iconBasePath + filename << std::endl;
            return false;
        }
        return true;
    };

    if (!loadTex(storageInTexture, "storage_in.png"))
        loadSuccess = false;
    if (!loadTex(storageOutTexture, "storage_out.png"))
        loadSuccess = false;
    if (!loadTex(workInTexture, "work_in.png"))
        loadSuccess = false;
    if (!loadTex(workOutTexture, "work_out.png"))
        loadSuccess = false;
    if (!loadTex(chargeTexture, "charge_station.png"))
        loadSuccess = false;

    if (!loadSuccess)
    {
        std::cout << "WarehouseRenderer - Using default white square for some device icons." << std::endl;
        sf::Image defaultImage;
        defaultImage.create(32, 32, sf::Color::White);
        defaultTexture.loadFromImage(defaultImage);
        if (storageInTexture.getSize().x == 0)
            storageInTexture = defaultTexture;
        if (storageOutTexture.getSize().x == 0)
            storageOutTexture = defaultTexture;
        if (workInTexture.getSize().x == 0)
            workInTexture = defaultTexture;
        if (workOutTexture.getSize().x == 0)
            workOutTexture = defaultTexture;
        if (chargeTexture.getSize().x == 0)
            chargeTexture = defaultTexture;
    }

    // m_iconTextures[gui::DeviceType::INPUT_STATION] = storageInTexture; // Changed to Core::DeviceType
    // m_iconTextures[gui::DeviceType::OUTPUT_STATION] = storageOutTexture;
    // m_iconTextures[gui::DeviceType::CORE_WORKSTATION_IN] = workInTexture;
    // m_iconTextures[gui::DeviceType::CORE_WORKSTATION_OUT] = workOutTexture;
    // m_iconTextures[gui::DeviceType::CHARGER] = chargeTexture;
    m_iconTextures[DeviceType::StorageIn] = storageInTexture;
    m_iconTextures[DeviceType::StorageOut] = storageOutTexture;
    m_iconTextures[DeviceType::WorkstationIn] = workInTexture;
    m_iconTextures[DeviceType::WorkstationOut] = workOutTexture;
    // Assuming CHARGER is not a Core::DeviceType, or needs mapping if it is.
    // If CHARGER is a specific type in Core::DeviceType, map it here.
    // For now, it's removed as it's not in the provided Core::DeviceType enum.

    if (defaultTexture.getSize().x > 0) // sf::Texture::getSize 是一个常量成员函数，返回纹理的大小，类型为 sf::Vector2u，表示纹理的宽度和高度（以像素为单位）
    {
        // m_iconTextures[gui::DeviceType::UNKNOWN_DEVICE_TYPE] = defaultTexture; // No UNKNOWN in Core::DeviceType
        // m_iconTextures[gui::DeviceType::WORK_STATION] = defaultTexture;    // Generic work station as fallback
        // m_iconTextures[gui::DeviceType::STORAGE_STATION] = defaultTexture; // Generic storage as fallback
        // Add fallbacks if necessary, perhaps by not inserting into map if specific types aren't found,
        // and then checking for key existence before drawing.
    }

    float trackOuterEdgeOffsetMm = m_trackRendererRef->getTrackWidth() / 2.0f;
    // float mmToPx = m_trackRendererRef->getMmToPxRatio(); // REMOVED - Will fetch fresh values in loop
    // float currentMmToPx = m_trackRendererRef->getMmToPxRatio(); // REMOVED - Will fetch fresh values in loop
    // float currentScaleFactor = m_trackRendererRef->getScaleFactor(); // REMOVED - Will fetch fresh values in loop

    for (const auto &layout : s_deviceLayouts)
    {
        WarehouseInterface interface_element;
        interface_element.id = layout.id;
        interface_element.type = layout.type;
        interface_element.positionCategory = layout.positionCategory;
        interface_element.widthMm = layout.visualDepthMm;   // Use full depth as width for rendering
        interface_element.depthMm = layout.visualWidthMm;   // Use full width as depth for rendering
        interface_element.coreType = layout.coreDeviceType; // Assign core type from layout

        sf::Vector2f trackCenterPointPx;
        float trackAngleRad; // 用于存储从TrackRenderer获取的轨道角度

        // 从TrackRenderer获取指定距离处的轨道点坐标和切线角度
        // 添加 m_worldOriginOffsetPx 作为第四个参数
        if (!m_trackRendererRef->getPointAndOrientationOnCenterLine(layout.trackDistanceMm, trackCenterPointPx, trackAngleRad, m_worldOriginOffsetPx))
        {
            std::cerr << "WarehouseRenderer Error: Could not get point and orientation for device ID "
                      << layout.id << " at distance " << layout.trackDistanceMm << std::endl;
            continue; // 跳过这个设备
        }

        float normalAngleRad;
        const float angle_tolerance = 0.01f;

        // 首先判断是否在弯道上。我们可以通过 trackAngleRad 是否接近水平或垂直来粗略判断。
        // 如果 trackAngleRad 既不接近0/PI (水平) 也不接近 PI/2, -PI/2 (垂直)，则认为是弯道。
        // 或者，更可靠的方法是 TrackRenderer 提供一个函数 isPointOnCurve(distanceMm)。
        // 暂时我们先用角度判断。
        bool onStraightHorizontal = (std::abs(trackAngleRad) < angle_tolerance ||
                                     std::abs(trackAngleRad - M_PI) < angle_tolerance ||
                                     std::abs(trackAngleRad + M_PI) < angle_tolerance);
        // bool onStraightVertical = (std::abs(trackAngleRad - M_PI_2) < angle_tolerance ||
        //                            std::abs(trackAngleRad + M_PI_2) < angle_tolerance);

        if (layout.positionCategory == WarehousePosition::TOP)
        {
            // 对于顶部的仓库，法线总是指向屏幕上方
            normalAngleRad = -M_PI / 2.0f; // 数学坐标系：指向负Y轴 (屏幕上方)
        }
        else // WarehousePosition::BOTTOM
        {
            // 对于底部的仓库，法线总是指向屏幕下方
            normalAngleRad = M_PI / 2.0f; // 数学坐标系：指向正Y轴 (屏幕下方)
        }

        // 修改下面这一行：
        // 原来的: interface_element.worldRotationDegrees = -(trackAngleRad + M_PI / 2.0f) * (180.0f / M_PI);
        // 新的:
        interface_element.worldRotationDegrees = -trackAngleRad * (180.0f / M_PI);

        float totalOffsetFromCenterlineMm = trackOuterEdgeOffsetMm + layout.offsetFromTrackEdgeMm * 0.0f + (layout.visualDepthMm / 2.0f);

        // Fetch current scaling factors from TrackRenderer for pixel calculations
        float currentMmToPx = m_trackRendererRef->getMmToPxRatio();
        float currentScaleFactor = m_trackRendererRef->getScaleFactor();
        sf::Vector2f offsetVectorPx(
            totalOffsetFromCenterlineMm * std::cos(normalAngleRad) * currentMmToPx * currentScaleFactor,
            totalOffsetFromCenterlineMm * std::sin(normalAngleRad) * currentMmToPx * currentScaleFactor);

        interface_element.worldCenterPx.x = trackCenterPointPx.x + offsetVectorPx.x;
        interface_element.worldCenterPx.y = trackCenterPointPx.y + offsetVectorPx.y; // Y-up offset to Y-down screen

        // 仓库本体的旋转仍然应该垂直于轨道切线
        // SFML 的 setRotation 使用度数，正值顺时针。
        // trackAngleRad 是数学坐标（Y向上，逆时针为正）。
        // 仓库通常平行于法线，或垂直于切线。如果仓库长边平行于轨道，则旋转角度与轨道切线相关。
        // 如果仓库长边垂直于轨道（像图示那样），它的方向应该是 normalAngleRad + PI/2 (或者 normalAngleRad - PI/2)
        // 或者更简单，直接使用 trackAngleRad，然后根据需要加90度。
        // 让我们假设仓库的“前部”或“开口”面向轨道，其“边”平行于轨道。
        // 那么仓库的旋转应该是使其“边”与轨道切线平行。
        // 如果仓库的局部坐标系X轴是其长度方向，那么旋转 trackAngleRad 即可。
        // 但从图上看，仓库是矩形，其较长的一边是垂直于轨道的。
        // 所以仓库的旋转应该是 normalAngleRad (使其X轴指向法线方向)
        // 或者 trackAngleRad + 90度 (使其Y轴平行于轨道，X轴垂直于轨道)

        // 从您的图示看，仓库的ID号是横向的，仓库本体是竖向的（垂直于轨道）。
        // 这意味着仓库的旋转应该是使其“宽度”边平行于轨道，“深度”边垂直于轨道。
        // 那么仓库的旋转角度应该是 trackAngleRad (数学角度) 加上或减去90度，使其长边垂直。
        // 或者说，仓库的“朝向”是法线方向。
        // 如果仓库的局部坐标系的+Y轴是其“深度”方向（指向轨道或远离轨道），+X轴是其“宽度”方向（平行轨道）。
        // 那么，仓库的旋转应该是 trackAngleRad。
        // 但您的图示仓库是“站立”的，其深度方向是垂直于轨道的。
        // 所以，仓库的旋转应该是 trackAngleRad + 90度 (或者 -90度)。
        // 让我们用 trackAngleRad 来表示仓库的“边”与X轴的夹角，然后SFML需要负值。
        // 并且，仓库的形状是垂直于轨道的，所以是 trackAngleRad + PI/2。
        interface_element.worldRotationDegrees = -(trackAngleRad + M_PI / 2.0f) * (180.0f / M_PI);

        float wPx = interface_element.widthMm * currentMmToPx * currentScaleFactor;
        float hPx = interface_element.depthMm * currentMmToPx * currentScaleFactor;
        sf::Transform tx;
        tx.translate(interface_element.worldCenterPx);
        tx.rotate(interface_element.worldRotationDegrees);
        sf::Vector2f p1 = tx.transformPoint(-wPx / 2.f, -hPx / 2.f);
        sf::Vector2f p2 = tx.transformPoint(wPx / 2.f, -hPx / 2.f);
        sf::Vector2f p3 = tx.transformPoint(wPx / 2.f, hPx / 2.f);
        sf::Vector2f p4 = tx.transformPoint(-wPx / 2.f, hPx / 2.f);
        float minX = std::min({p1.x, p2.x, p3.x, p4.x});
        float maxX = std::max({p1.x, p2.x, p3.x, p4.x});
        float minY = std::min({p1.y, p2.y, p3.y, p4.y});
        float maxY = std::max({p1.y, p2.y, p3.y, p4.y});
        interface_element.boundsPx = sf::FloatRect(minX, minY, maxX - minX, maxY - minY);

        // Initialize Core::DeviceState and Core::DeviceType within WarehouseInterface
        // interface_element.coreType is now set from layout.coreDeviceType above

        // Default construct Core::DeviceState, it will be updated by updateDeviceStates
        interface_element.coreState = DeviceState();

        m_interfaces.push_back(interface_element);

        sf::Text label;
        label.setFont(m_font);
        label.setString(std::to_string(layout.id));
        // label.setCharacterSize(10); // 您可以根据需要调整字符大小
        // Scale character size with zoom, ensuring a minimum readable size
        unsigned int baseCharSize = 10; // Base character size at scale 1.0
        unsigned int minCharSize = 8;   // Minimum character size (pixels)
        // Ensure currentScaleFactor is not zero or negative if it can be, though typically it's > 0
        float effectiveScaleFactorForText = std::max(0.1f, m_trackRendererRef->getScaleFactor()); // Fetch fresh scale factor for text
        unsigned int scaledCharSize = static_cast<unsigned int>(static_cast<float>(baseCharSize) * effectiveScaleFactorForText);
        label.setCharacterSize(std::max(minCharSize, scaledCharSize));
        label.setFillColor(sf::Color::Black);
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);

        // 获取仓库的像素高度，用于计算标签的垂直偏移
        // float hPx = interface_element.depthMm * mmToPx; // <--- 移除这一行，因为 hPx 在前面已经计算过了 (第242行附近)
        float labelOffsetY;

        if (layout.positionCategory == WarehousePosition::BOTTOM)
        {
            // 如果仓库在下方，将文字显示在仓库下方
            // hPx / 2.0f 将标签移动到仓库的下边缘
            // + 5.0f 是额外的向下偏移量，以避免与仓库边框重叠
            labelOffsetY = hPx / 2.0f + 5.0f;
        }
        else // WarehousePosition::TOP (或其他默认情况)
        {
            // 如果仓库在上方，将文字显示在仓库上方
            // -hPx / 2.0f 将标签移动到仓库的上边缘
            // - 5.0f 是额外的向上偏移量
            labelOffsetY = -hPx / 2.0f - 5.0f;
        }

        // 设置标签位置，X轴方向上与仓库中心对齐，Y轴方向上根据上述逻辑偏移
        label.setPosition(interface_element.worldCenterPx + sf::Vector2f(0, labelOffsetY));

        m_labels.push_back(label);
    }
}

void WarehouseRenderer::updateDeviceStates(const std::vector<DeviceBase *> &coreDevices)
{
    for (auto &gui_device : m_interfaces)
    { // Iterate over m_interfaces directly
        bool found = false;
        for (const auto *core_device_ptr : coreDevices)
        {
            if (core_device_ptr && core_device_ptr->m_id == gui_device.id)
            {
                // Since DeviceBase doesn't directly expose state, we need to get it from DeviceManager
                // For now, we'll assume the caller provides the correct state through a different mechanism
                // This method signature should be updated to accept DeviceManager or states directly
                found = true;
                break;
            }
        }
        if (!found)
        {
            // Optionally handle cases where a GUI device doesn't have a matching core device
            gui_device.coreState = DeviceState(); // Reset to default state
            // Consider logging a warning or setting a specific 'offline' visual state if applicable
        }
    }
}

void WarehouseRenderer::updateDeviceStates(const std::map<int, DeviceState> &deviceStates)
{
    for (auto &gui_device : m_interfaces)
    {
        auto it = deviceStates.find(gui_device.id);
        if (it != deviceStates.end())
        {
            gui_device.coreState = it->second;
        }
        else
        {
            // Reset to default state if device not found
            gui_device.coreState = DeviceState();
        }
    }
}

void WarehouseRenderer::setInputColor(const sf::Color &color)
{
    m_inputColor = color;
}

void WarehouseRenderer::setOutputColor(const sf::Color &color)
{
    m_outputColor = color;
}

const WarehouseRenderer::WarehouseInterface *WarehouseRenderer::getInterfaceAt(const sf::Vector2f &worldPositionPx) const
{
    for (const auto &interface_obj : m_interfaces)
    {
        if (interface_obj.boundsPx.contains(worldPositionPx))
        {
            return &interface_obj;
        }
    }
    return nullptr;
}

void WarehouseRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    // 绘制所有接口设备
    for (const auto &interface : m_interfaces)
    {
        drawInterface(target, states, interface);
    }

    // 绘制所有标签
    for (const auto &label : m_labels)
    {
        target.draw(label, states);
    }
}

void WarehouseRenderer::drawInterface(sf::RenderTarget &target, sf::RenderStates states,
                                      const WarehouseInterface &interface_obj) const
{
    if (!m_trackRendererRef)
        return; // Should not happen if initialized

    // Fetch current scaling factors from TrackRenderer for this draw call
    float currentMmToPx = m_trackRendererRef->getMmToPxRatio();
    float currentScaleFactor = m_trackRendererRef->getScaleFactor();
    float visualWidthPx = interface_obj.widthMm * currentMmToPx * currentScaleFactor;
    float visualDepthPx = interface_obj.depthMm * currentMmToPx * currentScaleFactor; // visualDepthMm is dimension perpendicular to track

    sf::RectangleShape body(sf::Vector2f(visualWidthPx, visualDepthPx));
    body.setOrigin(visualWidthPx / 2.0f, visualDepthPx / 2.0f);
    body.setPosition(interface_obj.worldCenterPx);
    body.setRotation(interface_obj.worldRotationDegrees);

    sf::Color bodyFillColor;
    sf::Color bodyBorderColor = m_borderColor;

    // Determine color based on Core::DeviceState
    if (interface_obj.coreState.is_transferring)
    {
        bodyFillColor = COLOR_BUSY; // Changed from COLOR_WORKING to COLOR_BUSY as per .hpp definition
    }
    else if (interface_obj.coreState.is_reserved)
    {
        bodyFillColor = COLOR_RESERVED; // Use the new COLOR_RESERVED
    }
    else if (interface_obj.coreState.has_goods)
    {
        bodyFillColor = COLOR_HAS_GOODS;
    }
    else
    {
        bodyFillColor = COLOR_IDLE;
    }

    // The old switch statement based on gui::DeviceStatus is no longer applicable.
    // Remove or comment out the old switch:
    /*
    switch (interface_obj.coreState.status) // This 'status' member does not exist in Core::DeviceState
    {
    // ... cases ...
    }
    */

    // Override with input/output specific colors if IDLE, for better visual distinction as per image
    if (!interface_obj.coreState.is_transferring && !interface_obj.coreState.is_reserved && !interface_obj.coreState.has_goods)
    {
        // Use the GUI specific type for visual distinction of input/output areas when idle
        bodyFillColor = (interface_obj.type == InterfaceType::INPUT) ? sf::Color(60, 180, 75, 180) : sf::Color(230, 85, 40, 180);
    }

    body.setFillColor(bodyFillColor);
    // body.setOutlineThickness(1.0f * mmToPx > 0.5f ? 1.0f * mmToPx : 0.5f); // Ensure outline is visible
    float outlineThickness = 1.0f * currentMmToPx * currentScaleFactor;
    body.setOutlineThickness(std::max(0.5f, outlineThickness)); // Ensure outline is visible with a minimum
    body.setOutlineColor(bodyBorderColor);

    target.draw(body, states);

    // Draw Icon
    sf::Texture iconTexture;
    auto it = m_iconTextures.find(interface_obj.coreType);
    if (it != m_iconTextures.end())
    {
        iconTexture = it->second;
    }
    else
    {
        // Handle case where icon for the coreType is not found, maybe use a default
        // This depends on how you handled defaultTexture loading for Core::DeviceType keys
        // For now, assume it might lead to a blank texture if not found and no default was mapped.
    }

    if (iconTexture.getSize().x > 0) // Check if texture is valid
    {
        sf::Sprite iconSprite(iconTexture);
        sf::FloatRect spriteBounds = iconSprite.getLocalBounds();
        iconSprite.setOrigin(spriteBounds.width / 2.0f, spriteBounds.height / 2.0f);
        iconSprite.setPosition(interface_obj.worldCenterPx);
        // iconSprite.setRotation(interface_obj.worldRotationDegrees); // Icons typically don't rotate

        // Scale icon to fit within the device, leaving some padding
        float padding = 0.8f; // Use 80% of space for icon
        float scaleX = (visualWidthPx * padding) / spriteBounds.width;
        float scaleY = (visualDepthPx * padding) / spriteBounds.height;
        float iconScale = std::min(scaleX, scaleY);

        // Prevent overly tiny icons or huge icons if device is oddly shaped
        // Adjust icon scale considering the global currentScaleFactor from TrackRenderer
        iconScale = std::max(0.1f, std::min(iconScale, 2.0f * currentScaleFactor));

        iconSprite.setScale(iconScale, iconScale);
        target.draw(iconSprite, states);
    }
}

void WarehouseRenderer::createLabel(sf::Text &text, const sf::Font &font, const std::string &content,
                                    float x, float y, float scale)
{
    text.setFont(font);
    text.setString(content);
    text.setCharacterSize(12 * scale);
    text.setFillColor(sf::Color::Black);

    // 设置阴影
    text.setOutlineColor(sf::Color(255, 255, 255, 180));
    text.setOutlineThickness(0.5f);

    // 居中对齐
    sf::FloatRect bounds = text.getLocalBounds();
    text.setOrigin(bounds.left + bounds.width / 2.0f, bounds.top + bounds.height / 2.0f);
    text.setPosition(x, y);
}
