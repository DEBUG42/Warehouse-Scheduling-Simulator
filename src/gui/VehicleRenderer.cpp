#include "gui/VehicleRenderer.hpp"
#include <cmath>    // For std::cos, std::sin, M_PI
#include <iostream> // For std::cerr

// Constructor
VehicleRenderer::VehicleRenderer(const sf::Font &font, TrackRenderer &trackRenderer)
    : m_font(font), m_trackRendererRef(&trackRenderer), m_vehicleVisualScale(1.0f) // Initialize m_vehicleVisualScale
{
    // Initialize shapes if needed, or do it in renderVehicle
    m_body.setOutlineThickness(1.f);     // Example initialization
    m_directionIndicator.setRadius(5.f); // Example
    m_idText.setFont(m_font);
    m_idText.setCharacterSize(10);
    m_idText.setFillColor(sf::Color::Black);
}

// Get color for status
sf::Color VehicleRenderer::getColorForStatus(gui::VehicleStatus status) const
{
    switch (status)
    {
    case gui::VehicleStatus::IDLE:
        return m_emptyColor; // Or a specific IDLE color if different from empty
    case gui::VehicleStatus::MOVING_TO_LOAD:
    case gui::VehicleStatus::MOVING_TO_UNLOAD:
        return m_colorAssigned; // Or a specific MOVING color
    case gui::VehicleStatus::LOADING:
    case gui::VehicleStatus::UNLOADING:
        return m_loadedColor; // Or specific LOADING/UNLOADING colors
    case gui::VehicleStatus::CHARGING:
        return sf::Color::Yellow; // Example for charging
    case gui::VehicleStatus::ERROR:
        return sf::Color::Red;
    default:
        return sf::Color(128, 128, 128); // Grey for UNKNOWN or other states
    }
}

// Calculate position and rotation of the vehicle on the track
void VehicleRenderer::calculatePosition(const gui::VehicleState &vehicle,
                                        TrackRenderer &trackRenderer, // Parameter kept for now
                                        const sf::Vector2f &worldOriginOffsetPx,
                                        sf::Vector2f &position, float &rotation)
{
    if (!m_trackRendererRef)
    {
        std::cerr << "VehicleRenderer Error: TrackRenderer reference not set!" << std::endl;
        position = vehicle.getPosition(); // Fallback to SimObject position
        rotation = 0.0f;                  // rotation is in radians
        return;
    }
    m_trackRendererRef->getPointAndOrientationOnCenterLine(vehicle.rawTrackPositionMm, position, rotation, worldOriginOffsetPx);
}

// Render a single vehicle
void VehicleRenderer::renderVehicle(sf::RenderTarget &target,
                                    const gui::VehicleState &vehicle,
                                    const sf::Vector2f &position,
                                    float rotationDegrees) const // Added const
{
    if (!m_trackRendererRef)
        return;

    float currentMmToPx = m_trackRendererRef->getMmToPxRatio();
    float currentScaleFactor = m_trackRendererRef->getScaleFactor() * m_vehicleVisualScale; // Combine global and vehicle-specific scale

    float vehWidthPx = m_vehicleWidth * currentMmToPx * currentScaleFactor;
    float vehLengthPx = m_vehicleLength * currentMmToPx * currentScaleFactor;

    m_body.setSize(sf::Vector2f(vehLengthPx, vehWidthPx));
    m_body.setOrigin(vehLengthPx / 2.f, vehWidthPx / 2.f);
    m_body.setPosition(position);
    m_body.setRotation(rotationDegrees); // Use degrees directly

    sf::Color fillColor = getColorForStatus(vehicle.status);
    if (vehicle.isLoaded)
    {
        fillColor = m_loadedColor;
    }
    m_body.setFillColor(fillColor);
    m_body.setOutlineColor(m_borderColor);
    // Make outline thickness scale with zoom but have a minimum
    float outlineThickness = 1.0f * currentMmToPx * currentScaleFactor;
    m_body.setOutlineThickness(std::max(0.5f, outlineThickness));

    target.draw(m_body);

    // Render ID text
    m_idText.setString(vehicle.getId());
    // Scale character size with zoom, ensuring a minimum readable size
    unsigned int baseCharSize = 10; // Base character size at scale 1.0
    unsigned int minCharSize = 8;   // Minimum character size (pixels)
    // Ensure currentScaleFactor is not zero or negative
    float effectiveScaleFactorForText = std::max(0.1f, currentScaleFactor);
    unsigned int scaledCharSize = static_cast<unsigned int>(static_cast<float>(baseCharSize) * effectiveScaleFactorForText);
    m_idText.setCharacterSize(std::max(minCharSize, scaledCharSize));

    sf::FloatRect textBounds = m_idText.getLocalBounds();
    m_idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    m_idText.setPosition(position); // Position in the center of the vehicle
    m_idText.setRotation(0);        // Text should not rotate with the vehicle, or apply counter-rotation if vehicle rotation is extreme
    target.draw(m_idText);

    // Optionally, render direction indicator, status bounds, etc.
    // Example for direction indicator (simple triangle)
    sf::ConvexShape directionArrow;
    directionArrow.setPointCount(3);
    directionArrow.setPoint(0, sf::Vector2f(vehLengthPx / 2.f, 0)); // Tip of the arrow at the front-center
    directionArrow.setPoint(1, sf::Vector2f(vehLengthPx / 2.f - vehWidthPx / 2.f, -vehWidthPx / 2.f));
    directionArrow.setPoint(2, sf::Vector2f(vehLengthPx / 2.f - vehWidthPx / 2.f, vehWidthPx / 2.f));
    directionArrow.setOrigin(vehLengthPx / 2.f, 0); // Origin at the front-center of the base of the arrow
    directionArrow.setPosition(position);           // Position at vehicle center
    directionArrow.setRotation(rotationDegrees);    // Rotate with vehicle
    directionArrow.setFillColor(sf::Color::White);
    // target.draw(directionArrow); // Uncomment to draw
}

// Render shadow (simplified)
void VehicleRenderer::renderShadow(sf::RenderTarget &target,
                                   const sf::Vector2f &position,
                                   float rotationDegrees) const // Added const
{
    if (!m_trackRendererRef)
        return;
    float currentMmToPx = m_trackRendererRef->getMmToPxRatio();
    float currentScaleFactor = m_trackRendererRef->getScaleFactor() * m_vehicleVisualScale;

    float vehWidthPx = m_vehicleWidth * currentMmToPx * currentScaleFactor;
    float vehLengthPx = m_vehicleLength * currentMmToPx * currentScaleFactor;

    sf::RectangleShape shadow = m_body; // Copy properties
    shadow.setSize(sf::Vector2f(vehLengthPx, vehWidthPx));
    shadow.setOrigin(vehLengthPx / 2.f, vehWidthPx / 2.f);
    shadow.setFillColor(m_shadowColor);
    // Offset shadow slightly
    shadow.setPosition(position.x + 3.f * currentScaleFactor, position.y + 3.f * currentScaleFactor);
    shadow.setRotation(rotationDegrees); // Use degrees directly
    target.draw(shadow);
}

// Update all vehicle states
void VehicleRenderer::updateVehicleStates(const std::vector<gui::VehicleState> &vehicles, const sf::Vector2f &worldOriginOffsetPx)
{
    m_vehicles.clear(); // Clear old states
    if (!m_trackRendererRef)
    {
        std::cerr << "VehicleRenderer Error: TrackRenderer reference not set for updating states!" << std::endl;
        // Optionally copy vehicles without updated screen coords if fallback is desired
        // m_vehicles = vehicles;
        return;
    }

    for (const auto &coreState : vehicles)
    {
        gui::VehicleState renderState = coreState; // Copy core data
        sf::Vector2f screenPos;
        float screenRotRad;
        // Assuming TrackRenderer has a way to get worldOriginOffsetPx or it's passed/known
        // For this example, using a default sf::Vector2f(0,0) for worldOriginOffsetPx
        // This should be replaced with the actual world origin offset used by TrackRenderer
        // sf::Vector2f worldOriginOffsetPx(0,0); // Placeholder - MUST BE SET CORRECTLY - Now passed as parameter
        if (m_trackRendererRef)
        { // Ensure ref is valid
            m_trackRendererRef->getPointAndOrientationOnCenterLine(
                coreState.rawTrackPositionMm,
                screenPos,
                screenRotRad,
                worldOriginOffsetPx // This needs to be the correct offset
            );
            renderState.setPosition(screenPos);                               // Update SimObject's position
            renderState.worldRotationDegrees = screenRotRad * (180.f / M_PI); // Store rotation in degrees
        }
        else
        {
            // Fallback if track renderer isn't available (e.g., use raw/default positions)
            renderState.setPosition(coreState.getPosition()); // Use original position from SimObject
            renderState.worldRotationDegrees = 0.0f;          // Default rotation
        }
        m_vehicles.push_back(renderState);
    }
}

// Main draw call for all vehicles
void VehicleRenderer::draw(sf::RenderTarget &target, sf::RenderStates states) const
{
    if (!m_trackRendererRef)
    {
        std::cerr << "VehicleRenderer Error: TrackRenderer reference not set for drawing!" << std::endl;
        return;
    }
    states.transform *= getTransform(); // Apply VehicleRenderer\'s own transform if any

    for (const auto &vehicle : m_vehicles)
    {
        // VehicleState should now have pre-calculated screen position and rotation
        sf::Vector2f vehicleScreenPos = vehicle.getPosition();   // This is SimObject.m_position
        float vehicleRotationDeg = vehicle.worldRotationDegrees; // Use stored rotation

        renderShadow(target, vehicleScreenPos, vehicleRotationDeg);           // Shadow first
        renderVehicle(target, vehicle, vehicleScreenPos, vehicleRotationDeg); // Then vehicle
    }
}

// Update a single vehicle's state, including calculating its screen position and rotation
void VehicleRenderer::updateState(const gui::VehicleState &state, TrackRenderer &trackRenderer, const sf::Vector2f &worldOriginOffsetPx)
{
    // This function might be for a single focused vehicle, or could be part of a larger update loop.
    // For consistency, it should ideally update a vehicle within m_vehicles or m_currentState if it's drawn separately.
    // Let's assume it updates m_currentState for now, and m_currentState is drawn specially or is a reference to one in m_vehicles.
    m_currentState = state; // Store the new state
    sf::Vector2f screenPos;
    float screenRotRad;

    // Use the provided trackRenderer and worldOriginOffsetPx for this specific update
    calculatePosition(m_currentState, trackRenderer, worldOriginOffsetPx, screenPos, screenRotRad);

    m_currentState.setPosition(screenPos);                               // Updates the m_position in SimObject base
    m_currentState.worldRotationDegrees = screenRotRad * (180.f / M_PI); // Store calculated rotation in degrees

    // If m_currentState is one of the vehicles in m_vehicles, that specific element should be updated.
    // Example: if m_currentState.getId() is used to find and update in m_vehicles:
    // for (auto& v : m_vehicles) {
    //     if (v.getId() == m_currentState.getId()) {
    //         v.setPosition(screenPos);
    //         v.worldRotationDegrees = screenRotRad * (180.f / M_PI);
    //         break;
    //     }
    // }
}

void VehicleRenderer::setVehicleVisualScale(float scale)
{
    m_vehicleVisualScale = std::max(0.1f, scale); // Ensure scale is positive
}

// Note: The logic in `draw` regarding recalculating rotation is a temporary workaround.
// The best solution is to ensure `VehicleState` (or a parallel structure) holds all necessary
// pre-calculated rendering information (screen position, screen rotation) before `draw` is called.
// This is typically done in an update phase that calls `calculatePosition` for each vehicle.