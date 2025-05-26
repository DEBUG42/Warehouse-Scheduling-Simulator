#include "gui/WarehouseRenderer.hpp"
#include <iostream>
#include <cmath> // For std::cos, std::sin, M_PI

/*
每一个仓库对应在弯道上的位置，
根据开发界面信息规定的将整个轨道左下角弯道与直道的交汇点作为原点，
计算出每个仓库接口应该对应的位置

弯道半径=2500mm
pi=3.14159265358979323846
18 8000mm
17 11000mm
16 14000mm
15 26000mm
14 29000mm
13 32000mm
12 40000+2500pi+3800=43800+7500pi=   51635.981634mm
11 40000+2500pi+6200=46200+7500pi=   54035.981634mm
10 40000+2500pi+9800=49800+7500pi=   57635.981634mm
9  40000+2500pi+12200=52200+7500pi=   60035.981634mm
8  40000+2500pi+15800=55800+7500pi=   63635.981634mm
7  40000+2500pi+18200=58200+7500pi=   66035.981634mm
6  40000+2500pi+21800=61800+7500pi=   69635.981634mm
5  40000+2500pi+24200=64200+7500pi=   72035.981634mm
4  40000+2500pi+27800=67800+7500pi=   75635.981634mm
3  40000+2500pi+30200=70200+7500pi=   78035.981634mm
2  40000+2500pi+33800=73800+7500pi=   81635.981634mm
1  40000+2500pi+36200=76200+7500pi=   84035.981634mm
*/

// Define the fixed layout based on comments and image from user
const std::vector<WarehouseRenderer::PredefinedDeviceLayout> WarehouseRenderer::s_deviceLayouts = {
    // Bottom track devices (IDs 13-18)
    // Device ID, TrackDist MM, Type, PositionHint, VisualWidth MM, VisualDepth MM, OffsetFromTrackEdge MM
    {18, 8000.0f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1000, 1500, 200},
    {17, 11000.0f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1000, 1500, 200},
    {16, 14000.0f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1000, 1500, 200},
    {15, 26000.0f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1000, 1500, 200},
    {14, 29000.0f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1000, 1500, 200},
    {13, 32000.0f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::BOTTOM, 1000, 1500, 200},

    // Top track devices (IDs 1-12)
    // User provided distances: 12:51635.98, 11:54035.98, 10:57635.98, 9:60035.98, 8:63635.98, 7:66035.98, 6:69635.98, 5:72035.98, 4:75635.98, 3:78035.98, 2:81635.98, 1:84035.98
    // Image: Odd IDs are INPUT_STATION (arrow in), Even IDs are OUTPUT_STATION (arrow out) for top row.
    {12, 51635.981634f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {11, 54035.981634f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {10, 57635.981634f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {9, 60035.981634f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {8, 63635.981634f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {7, 66035.981634f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {6, 69635.981634f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {5, 72035.981634f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {4, 75635.981634f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {3, 78035.981634f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {2, 81635.981634f, WarehouseRenderer::InterfaceType::OUTPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
    {1, 84035.981634f, WarehouseRenderer::InterfaceType::INPUT, WarehouseRenderer::WarehousePosition::TOP, 1000, 1500, 200},
};

WarehouseRenderer::WarehouseRenderer()
{
    if (!m_font.loadFromFile("assets/fonts/Arial.ttf"))
    {
        if (!m_font.loadFromFile("resources/fonts/arial.ttf"))
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

    m_iconTextures[gui::DeviceType::INPUT_STATION] = storageInTexture;
    m_iconTextures[gui::DeviceType::OUTPUT_STATION] = storageOutTexture;
    m_iconTextures[gui::DeviceType::CORE_WORKSTATION_IN] = workInTexture;
    m_iconTextures[gui::DeviceType::CORE_WORKSTATION_OUT] = workOutTexture;
    m_iconTextures[gui::DeviceType::CHARGER] = chargeTexture;
    if (defaultTexture.getSize().x > 0) // sf::Texture::getSize 是一个常量成员函数，返回纹理的大小，类型为 sf::Vector2u，表示纹理的宽度和高度（以像素为单位）
    {
        m_iconTextures[gui::DeviceType::UNKNOWN_DEVICE_TYPE] = defaultTexture;
        m_iconTextures[gui::DeviceType::WORK_STATION] = defaultTexture;    // Generic work station as fallback
        m_iconTextures[gui::DeviceType::STORAGE_STATION] = defaultTexture; // Generic storage as fallback
    }

    float trackOuterEdgeOffsetMm = m_trackRendererRef->getTrackWidth() / 2.0f;
    float mmToPx = m_trackRendererRef->getMmToPxRatio();

    for (const auto &layout : s_deviceLayouts)
    {
        WarehouseInterface interface_element; // Renamed to avoid conflict with member name
        interface_element.id = layout.id;
        interface_element.type = layout.type;
        interface_element.positionCategory = layout.positionCategory;
        interface_element.widthMm = layout.visualWidthMm;
        interface_element.depthMm = layout.visualDepthMm;

        sf::Vector2f trackCenterPointPx;
        float trackAngleRad;

        if (!m_trackRendererRef->getPointAndOrientationOnCenterLine(layout.trackDistanceMm, trackCenterPointPx, trackAngleRad, m_worldOriginOffsetPx))
        {
            std::cerr << "WarehouseRenderer Error: Could not get track point for device ID " << layout.id << " at " << layout.trackDistanceMm << "mm." << std::endl;
            continue;
        }

        float normalAngleRad = trackAngleRad - (M_PI / 2.0f); // Default normal (e.g., "right" or "down" side of track travel)

        // Adjust normal direction based on position category
        if (layout.positionCategory == WarehousePosition::TOP) {
            normalAngleRad += M_PI; // Flip direction for TOP devices (e.g. "left" or "up" side)
        }

        float totalOffsetFromCenterlineMm = trackOuterEdgeOffsetMm + layout.offsetFromTrackEdgeMm + (layout.visualDepthMm / 2.0f);
        sf::Vector2f offsetVectorPx(
            totalOffsetFromCenterlineMm * std::cos(normalAngleRad) * mmToPx,
            totalOffsetFromCenterlineMm * std::sin(normalAngleRad) * mmToPx); // offsetVectorPx.y is a Y-up delta

        // trackCenterPointPx is Y-down (render coordinates from TrackRenderer)
        // offsetVectorPx is calculated with Y-up math conventions (sin gives Y-up delta for y)
        // To apply a Y-up y-offset to a Y-down base y-coordinate, we subtract the offset's y-component.
        interface_element.worldCenterPx.x = trackCenterPointPx.x + offsetVectorPx.x;
        interface_element.worldCenterPx.y = trackCenterPointPx.y - offsetVectorPx.y; 

        // trackAngleRad is Y-up (CCW from +X axis is positive)
        // SFML's setRotation uses degrees, with positive values rotating clockwise.
        // Therefore, the Y-up angle must be negated for SFML.
        interface_element.worldRotationDegrees = -trackAngleRad * (180.0f / M_PI); 

        float wPx = interface_element.widthMm * mmToPx;
        float hPx = interface_element.depthMm * mmToPx;
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

        // Initialize gui::DeviceState within WarehouseInterface
        gui::DeviceType determinedType = (layout.type == WarehouseRenderer::InterfaceType::INPUT) ? gui::DeviceType::INPUT_STATION : gui::DeviceType::OUTPUT_STATION;
        // TODO: Further refine 'determinedType' based on s_deviceLayouts if it has more specific info (e.g. WORKSTATION types)
        // For now, all are INPUT_STATION or OUTPUT_STATION based on InterfaceType.

        interface_element.state = gui::DeviceState(
            std::to_string(layout.id),
            interface_element.worldCenterPx,
            determinedType,
            gui::DeviceStatus::IDLE // Default status
        );

        m_interfaces.push_back(interface_element);

        sf::Text label;
        label.setFont(m_font);
        label.setString(std::to_string(layout.id));
        label.setCharacterSize(10);
        label.setFillColor(sf::Color::Black);
        sf::FloatRect textBounds = label.getLocalBounds();
        label.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
        label.setPosition(interface_element.worldCenterPx + sf::Vector2f(0, -hPx / 2 - 5));
        m_labels.push_back(label);
    }
}

void WarehouseRenderer::updateDeviceStates(const std::vector<gui::DeviceState> &deviceStates)
{
    // 更新每个接口设备的状态
    for (const auto &state : deviceStates)
    {
        for (auto &interface : m_interfaces)
        {
            if (std::to_string(interface.id) == state.getId())
            {
                interface.state = state;
                break;
            }
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

    float mmToPx = m_trackRendererRef->getMmToPxRatio();
    float visualWidthPx = interface_obj.widthMm * mmToPx;
    float visualDepthPx = interface_obj.depthMm * mmToPx; // visualDepthMm is dimension perpendicular to track

    sf::RectangleShape body(sf::Vector2f(visualWidthPx, visualDepthPx));
    body.setOrigin(visualWidthPx / 2.0f, visualDepthPx / 2.0f);
    body.setPosition(interface_obj.worldCenterPx);
    body.setRotation(interface_obj.worldRotationDegrees);

    sf::Color bodyFillColor;
    sf::Color bodyBorderColor = m_borderColor;

    switch (interface_obj.state.status)
    {
    case gui::DeviceStatus::IDLE:
        bodyFillColor = COLOR_IDLE;
        break;
    case gui::DeviceStatus::WORKING:
        bodyFillColor = COLOR_WORKING;
        break;
    case gui::DeviceStatus::BUSY:
        bodyFillColor = COLOR_WORKING;
        break; // Map BUSY to WORKING
    case gui::DeviceStatus::ERROR:
        bodyFillColor = COLOR_FAULT;
        bodyBorderColor = sf::Color::Red;
        break;
    case gui::DeviceStatus::OFFLINE:
        bodyFillColor = COLOR_OFFLINE;
        break;
    default:
        bodyFillColor = sf::Color(100, 100, 100, 150); // Default fallback
    }

    // Override with input/output specific colors if IDLE, for better visual distinction as per image
    if (interface_obj.state.status == gui::DeviceStatus::IDLE)
    {
        // These m_inputColor/m_outputColor are from WarehouseRenderer members, ensure they are set if used.
        // Or, define them locally here. The image implies distinct colors for input/output stations.
        // Let's use the member ones, assuming they are set to something reasonable like green/blue or green/red.
        // From original file: m_inputColor{60, 180, 75}; m_outputColor{230, 85, 40};
        bodyFillColor = (interface_obj.type == InterfaceType::INPUT) ? sf::Color(60, 180, 75, 180) : sf::Color(230, 85, 40, 180);
    }

    body.setFillColor(bodyFillColor);
    body.setOutlineThickness(1.0f * mmToPx > 0.5f ? 1.0f * mmToPx : 0.5f); // Ensure outline is visible
    body.setOutlineColor(bodyBorderColor);

    target.draw(body, states);

    // Draw Icon
    auto it = m_iconTextures.find(interface_obj.state.deviceType);
    if (it != m_iconTextures.end() && it->second.getSize().x > 0)
    { // Check if texture is valid
        sf::Sprite iconSprite(it->second);
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
        iconScale = std::max(0.1f, std::min(iconScale, 2.0f));

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
