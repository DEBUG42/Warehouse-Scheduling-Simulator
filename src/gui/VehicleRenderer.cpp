#define _USE_MATH_DEFINES // For M_PI in MSVC
#include <cmath>          // For M_PI, std::abs, std::cos, std::sin
#include "gui/VehicleRenderer.hpp"
#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/ConvexShape.hpp>
#include <iostream> // For debug output
// #include "Core/Vehicle.hpp" // Already included via VehicleRenderer.hpp and types are brought in with `using`

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
VehicleRenderer::VehicleRenderer(const sf::Font &font, TrackRenderer &trackRenderer)
    : m_font(font), m_trackRendererRef(&trackRenderer), m_vehicleVisualScale(1.0f)
{
    m_body.setOutlineThickness(1.f);
    m_directionIndicator.setRadius(5.f);
    m_idText.setFont(m_font);
    m_idText.setCharacterSize(10);
    m_idText.setFillColor(sf::Color::Black);
}

void VehicleRenderer::initializeTexture(const std::string &texturePath, float scale)
{
    if (!m_vehicleTexture.loadFromFile(texturePath))
    {
        std::cerr << "Error loading vehicle texture: " << texturePath << std::endl;
        return;
    }
    m_vehicleSprite.setTexture(m_vehicleTexture);
    m_visualScale = scale;
    m_vehicleSprite.setScale(m_visualScale, m_visualScale);
    m_vehicleSprite.setOrigin(m_vehicleTexture.getSize().x / 2.0f, m_vehicleTexture.getSize().y / 2.0f);
    m_initialized = true;
}

sf::Color VehicleRenderer::getColorForMotionState(Vehicle::MotionState motionState) const // Use MotionState directly
{
    switch (motionState)
    {
    case Vehicle::MotionState::Stopped:
        return m_emptyColor;
    case Vehicle::MotionState::Accelerating:
    case Vehicle::MotionState::Cruising:
    case Vehicle::MotionState::Decelerating:
        return m_colorAssigned;
    default:
        return sf::Color(128, 128, 128); // Grey for UNKNOWN or other states
    }
}

sf::Color VehicleRenderer::getColorForVehicleState(const Vehicle *vehicle) const // Use VehicleState directly
{
    // 穿梭车无任务无货、有任务无货、有货等不同状态的显示需不同
    //  These are mostly covered by is_loaded or motion state. If specific colors are needed for PICKING/DROPPING:
    //  sf::Color m_colorIdleNoCargo{120, 180, 240};    // 无任务无货（浅蓝）
    //  sf::Color m_colorAssignedNoCargo{255, 200, 60}; // 有任务无货（橙黄）
    //  sf::Color m_colorLoaded{230, 85, 40};           // 有货（橙红）
    if (vehicle->m_state.currentTask == nullptr && !vehicle->is_loaded)
    {
        return m_colorIdleNoCargo; // 无任务无货
    }
    else if (vehicle->m_state.currentTask != nullptr && !vehicle->is_loaded)
    {
        return m_colorAssignedNoCargo; // 有任务无货
    }
    else if (vehicle->is_loaded)
    {
        return m_colorLoaded; // 有货
    }
    else
    {
        return m_colorError; // Default error color if no other state matches
    }
}

void VehicleRenderer::calculateScreenPositionAndRotation(const Vehicle &vehicle, // Use Vehicle directly
                                                         TrackRenderer &trackRenderer,
                                                         const sf::Vector2f &worldOriginOffsetPx,
                                                         sf::Vector2f &screenPosition,
                                                         float &screenRotationDegrees) const // Added const
{
    if (!m_trackRendererRef)
    {
        std::cerr << "VehicleRenderer Error: TrackRenderer reference not set!" << std::endl;
        screenPosition = sf::Vector2f(0, 0);
        screenRotationDegrees = 0.0f;
        return;
    }
    float rotationRadians = 0.0f;
    // Pass `this->m_trackRendererRef` instead of the parameter `trackRenderer` if it's meant to use the member.
    // However, the current signature allows overriding, which might be intentional for some scenarios.
    // For now, using the passed `trackRenderer` as per signature.
    trackRenderer.getPointAndOrientationOnCenterLine(vehicle.position_m, screenPosition, rotationRadians, worldOriginOffsetPx);
    screenRotationDegrees = rotationRadians * (180.f / M_PI);
}

void VehicleRenderer::renderSingleVehicle(sf::RenderTarget &target,
                                          const Vehicle &vehicle, // Use Vehicle directly
                                          const sf::Vector2f &screenPosition,
                                          float screenRotationDegrees) const
{
    if (!m_trackRendererRef)
        return;

    float currentMmToPx = m_trackRendererRef->getMmToPxRatio();
    float currentScaleFactor = m_trackRendererRef->getScaleFactor() * m_vehicleVisualScale;

    float vehWidthPx = m_vehicleWidth * currentMmToPx * currentScaleFactor;
    float vehLengthPx = m_vehicleLength * currentMmToPx * currentScaleFactor;

    m_body.setSize(sf::Vector2f(vehLengthPx, vehWidthPx));
    m_body.setOrigin(vehLengthPx / 2.f, vehWidthPx / 2.f);
    m_body.setPosition(screenPosition);
    m_body.setRotation(screenRotationDegrees);
    sf::Color fillColor = getColorForMotionState(vehicle.m_state.motionState);
    if (vehicle.is_loaded) // Directly use Vehicle::is_loaded
    {
        fillColor = m_colorLoaded; // Use member variable instead of m_loadedColor
    }

    // Override with specific vehicle state color if applicable (e.g., for PICKING, DROPPING)
    sf::Color vehicleStateColor = getColorForVehicleState(&vehicle); // Get color based on vehicle state
    if (vehicleStateColor != sf::Color::Transparent)
    {
        fillColor = vehicleStateColor;
    }

    m_body.setFillColor(fillColor);
    m_body.setOutlineColor(m_borderColor);
    float outlineThickness = 1.0f * currentMmToPx * currentScaleFactor; // Scale outline
    m_body.setOutlineThickness(std::max(0.5f, outlineThickness));       // Ensure minimum thickness

    target.draw(m_body);

    // Render ID text
    m_idText.setString(std::to_string(vehicle.id)); // Use Vehicle::id
    unsigned int baseCharSize = 10;
    unsigned int minCharSize = 8;
    float effectiveScaleFactorForText = std::max(0.1f, currentScaleFactor);
    unsigned int scaledCharSize = static_cast<unsigned int>(static_cast<float>(baseCharSize) * effectiveScaleFactorForText);
    m_idText.setCharacterSize(std::max(minCharSize, scaledCharSize));

    sf::FloatRect textBounds = m_idText.getLocalBounds();
    m_idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    m_idText.setPosition(screenPosition);
    m_idText.setRotation(0);
    target.draw(m_idText);

    // Draw battery and cargo (if these helpers are adapted for Vehicle)
    // For now, assuming Vehicle has float batteryLevel and a way to check cargo for display
    // drawBatteryIndicator(target, vehicle, screenPosition, vehHeightPx, currentScaleFactor); // vehHeightPx needs to be defined
    // drawCargoIndicator(target, vehicle, screenPosition, vehWidthPx, vehHeightPx, currentScaleFactor);
}

void VehicleRenderer::renderShadow(sf::RenderTarget &target,
                                   const sf::Vector2f &screenPosition,
                                   float screenRotationDegrees) const
{
    if (!m_trackRendererRef)
        return;
    float currentMmToPx = m_trackRendererRef->getMmToPxRatio();
    float currentScaleFactor = m_trackRendererRef->getScaleFactor() * m_vehicleVisualScale;

    float vehWidthPx = m_vehicleWidth * currentMmToPx * currentScaleFactor;
    float vehLengthPx = m_vehicleLength * currentMmToPx * currentScaleFactor;

    sf::RectangleShape shadow = m_body;
    shadow.setSize(sf::Vector2f(vehLengthPx, vehWidthPx));
    shadow.setOrigin(vehLengthPx / 2.f, vehWidthPx / 2.f);
    shadow.setFillColor(m_shadowColor);
    shadow.setPosition(screenPosition.x + 3.f * currentScaleFactor, screenPosition.y + 3.f * currentScaleFactor);
    shadow.setRotation(screenRotationDegrees);
    target.draw(shadow);
}

void VehicleRenderer::setVehiclesToRender(const std::vector<Vehicle *> &vehicles)
{
    m_vehicles.clear();
    for (const auto *vehicle_ptr : vehicles)
    {
        if (vehicle_ptr)
        { // Ensure the pointer is not null
            // No const_cast needed if m_vehicles stores const Vehicle* or if draw() handles const Vehicle*
            m_vehicles.push_back(const_cast<Vehicle *>(vehicle_ptr));
        }
    }
}

void VehicleRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!m_trackRendererRef)
        return;
    states.transform *= getTransform();

    sf::View currentView = target.getView();
    float zoomLevel = m_trackRendererRef->getCurrentZoomLevel();
    sf::Vector2f worldOriginOffsetPx = m_trackRendererRef->getWorldOriginOffset();

    for (const Vehicle *vehicle_ptr : m_vehicles)
    { // Iterate over const Vehicle*
        if (!vehicle_ptr)
            continue;
        const Vehicle &vehicle = *vehicle_ptr;

        sf::Vector2f screenPos;
        float screenRotation;
        // Pass *m_trackRendererRef to calculateScreenPositionAndRotation
        // No const_cast needed as calculateScreenPositionAndRotation is now const
        calculateScreenPositionAndRotation(vehicle, *m_trackRendererRef, worldOriginOffsetPx, screenPos, screenRotation);

        renderShadow(target, screenPos, screenRotation);
        renderSingleVehicle(target, vehicle, screenPos, screenRotation);
    }
}

void VehicleRenderer::setVehicleVisualScale(float scale)
{
    m_vehicleVisualScale = std::max(0.1f, scale);
}

// Dummy implementations for new helper functions, adapt as needed
void VehicleRenderer::drawBatteryIndicator(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &screenPos, float spriteHeight, float zoomLevel) const
{
    // Assuming Vehicle has a batteryLevel member (e.g., vehicle.battery_level)
    // This is a placeholder - adapt to actual Vehicle structure
    float batteryLevel = 1.0f; // Example: vehicle.battery_level_percent / 100.0f;
    if (true)
    { // Replace true with a check if vehicle has battery info
        float barWidth = 20.0f / zoomLevel;
        float barHeight = 5.0f / zoomLevel;
        float yOffset = (spriteHeight / 2.0f + barHeight * 1.5f) / zoomLevel;

        sf::RectangleShape batteryBackground(sf::Vector2f(barWidth, barHeight));
        batteryBackground.setFillColor(sf::Color(100, 100, 100));
        batteryBackground.setPosition(screenPos.x - barWidth / 2.0f, screenPos.y + yOffset);

        float batteryFillWidth = barWidth * batteryLevel;
        sf::RectangleShape batteryFill(sf::Vector2f(batteryFillWidth, barHeight));
        batteryFill.setFillColor(batteryLevel > 0.2f ? sf::Color::Green : sf::Color::Red);
        batteryFill.setPosition(screenPos.x - barWidth / 2.0f, screenPos.y + yOffset);

        target.draw(batteryBackground);
        target.draw(batteryFill);
    }
}

void VehicleRenderer::drawCargoIndicator(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &screenPos, float spriteWidth, float spriteHeight, float zoomLevel) const
{
    if (!vehicle.is_loaded) // Use Vehicle::is_loaded
        return;

    float indicatorSize = 10.0f / zoomLevel;
    sf::CircleShape cargoIndicator(indicatorSize / 2.f);
    cargoIndicator.setFillColor(sf::Color::Magenta);
    cargoIndicator.setOrigin(indicatorSize / 2.f, indicatorSize / 2.f);
    cargoIndicator.setPosition(screenPos.x, screenPos.y - spriteHeight / 2.f - indicatorSize / 2.f - 2.f / zoomLevel);
    target.draw(cargoIndicator);

    // If Vehicle::m_state.currentTask stores task info with material_id:
    if (vehicle.m_state.currentTask && !vehicle.m_state.currentTask->material_id.empty())
    {
        sf::Text text(vehicle.m_state.currentTask->material_id, m_font, static_cast<unsigned int>(10.0 / zoomLevel));
        text.setFillColor(sf::Color::Black);
        text.setOrigin(text.getLocalBounds().width / 2, text.getLocalBounds().height / 2 + text.getLocalBounds().top);
        text.setPosition(cargoIndicator.getPosition().x, cargoIndicator.getPosition().y - indicatorSize / 1.5f);
        target.draw(text);
    }
}

void VehicleRenderer::drawVehicleID(sf::RenderTarget &target, const Vehicle &vehicle, const sf::Vector2f &screenPos, float spriteHeight, float zoomLevel) const
{
    sf::Text text(std::to_string(vehicle.id), m_font, static_cast<unsigned int>(12.0f / zoomLevel));
    text.setFillColor(sf::Color::Black);
    text.setOrigin(text.getLocalBounds().left + text.getLocalBounds().width / 2.0f,
                   text.getLocalBounds().top + text.getLocalBounds().height / 2.0f);
    text.setPosition(screenPos.x, screenPos.y);
    target.draw(text);
}

// Removed old updateVehicleStates, updateState, calculatePosition (now calculateScreenPositionAndRotation)
// Removed old renderVehicle, renderShadow (now renderSingleVehicle, renderShadow taking screen coords)
// getColorForStatus renamed to getColorForMotionState
// getColorForCoreState renamed to getColorForVehicleState