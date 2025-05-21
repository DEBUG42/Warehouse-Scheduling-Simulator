// Main file for the integrated simulation view
#include <SFML/Graphics.hpp>
// #include <TGUI/TGUI.hpp>                  // Assuming TGUI is available and this is a common include
// #include <TGUI/Backend/SFML-Graphics.hpp> // For TGUI SFML backend
#include <iostream>
#include <vector>
#include <string>
#include <sstream>        // Required for std::ostringstream
#include <cmath>          // For M_PI, cos, sin
#define _USE_MATH_DEFINES // For M_PI in MSVC, good practice

// Placeholder for WarehouseDisplay class (definition would go in a separate .hpp/.cpp)
class WarehouseDisplay : public sf::Drawable
{
private:
    struct WarehouseInstance
    {
        int id;
        sf::RectangleShape shape;
        sf::Text idText;
    };

public:
    WarehouseDisplay(const sf::Font &font) : m_font(font) {}

    void addWarehouse(int id, sf::Vector2f positionMm, sf::Vector2f sizeMm, bool isOutbound /*, sf::Color color = sf::Color::Red */)
    {
        WarehouseInstance wh;
        wh.id = id;

        wh.shape.setSize(sizeMm);
        // wh.shape.setFillColor(color); // Could use color parameter later for status
        wh.shape.setFillColor(isOutbound ? sf::Color(200, 0, 0) : sf::Color(0, 150, 0)); // Dark Red for Outbound, Dark Green for Inbound (example)
        wh.shape.setOrigin(sizeMm / 2.f);
        wh.shape.setPosition(positionMm);

        wh.idText.setFont(m_font);
        wh.idText.setString("ID: " + std::to_string(wh.id));
        wh.idText.setCharacterSize(12);
        wh.idText.setFillColor(sf::Color::White);
        sf::FloatRect textRect = wh.idText.getLocalBounds();
        wh.idText.setOrigin(textRect.left + textRect.width / 2.0f, textRect.top + textRect.height / 2.0f);
        wh.idText.setPosition(positionMm.x, positionMm.y - sizeMm.y / 2.f - 15.f);

        m_warehouses.push_back(wh);
        std::cout << "Added warehouse ID: " << id << " at (" << positionMm.x << "," << positionMm.y << ")" << std::endl;
    }

    void setMmToPxRatio(float ratio) { m_mmToPxRatio = ratio; }
    void setScaleFactor(float scale) { m_scaleFactor = scale; }

private:
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        for (const auto &wh_instance : m_warehouses)
        {
            target.draw(wh_instance.shape, states);
            target.draw(wh_instance.idText, states);
        }
    }
    float m_mmToPxRatio = 0.1f;
    float m_scaleFactor = 1.0f;
    const sf::Font &m_font;
    std::vector<WarehouseInstance> m_warehouses;
};

// Placeholder for TrackDisplay class
class TrackDisplay : public sf::Drawable
{
public:
    TrackDisplay() : m_trackWidthMm(1200.f),
                     m_trackColor(0, 0, 255), // Bright Blue for high visibility
                     m_arrowColor(255, 255, 0)
    {
        m_innerTrack.setPrimitiveType(sf::LineStrip);
        m_outerTrack.setPrimitiveType(sf::LineStrip);
        // m_directionArrows.setPrimitiveType(sf::Triangles); // Arrows temporarily disabled
    }

    void setTrackWidthMm(float width)
    {
        m_trackWidthMm = width;
    }

    void setTrackColor(const sf::Color &color)
    {
        m_trackColor = color;
    }
    void setArrowColor(const sf::Color &color)
    {
        m_arrowColor = color;
    }

    void generateTrack(float trackLengthMm, float curveRadiusMm)
    {
        m_innerTrack.clear();
        m_outerTrack.clear();
        // m_directionArrows.clear(); // Arrows temporarily disabled

        if (trackLengthMm <= 0 || curveRadiusMm <= 0 || m_trackWidthMm <= 0)
        {
            std::cerr << "TrackDisplay: Invalid parameters for generateTrack." << std::endl;
            return;
        }
        std::cout << "TrackDisplay: Generating track with L=" << trackLengthMm
                  << ", R=" << curveRadiusMm << ", W=" << m_trackWidthMm << std::endl;

        const float trackOffset = m_trackWidthMm / 2.0f;
        const int curveSegments = 36;
        const float angleIncrement = static_cast<float>(M_PI) / curveSegments;

        const float R_in = curveRadiusMm - trackOffset;
        const float R_out = curveRadiusMm + trackOffset;

        // Path points for the centerline of the track
        std::vector<sf::Vector2f> centerPath;
        // Left Curve (center (0,0), from PI/2 down to -PI/2)
        for (int i = 0; i <= curveSegments; ++i)
        {
            float angle = static_cast<float>(M_PI_2) - i * angleIncrement;
            centerPath.push_back({curveRadiusMm * cos(angle), curveRadiusMm * sin(angle)});
        }
        // Bottom Straight (from (0, -curveRadiusMm) to (trackLengthMm, -curveRadiusMm))
        centerPath.push_back({trackLengthMm, -curveRadiusMm});

        // Right Curve (center (trackLengthMm, 0), from -PI/2 up to PI/2)
        for (int i = 0; i <= curveSegments; ++i)
        {
            float angle = -static_cast<float>(M_PI_2) + i * angleIncrement;
            centerPath.push_back({trackLengthMm + curveRadiusMm * cos(angle), curveRadiusMm * sin(angle)});
        }
        // Top Straight (from (trackLengthMm, curveRadiusMm) to (0, curveRadiusMm))
        centerPath.push_back({0.f, curveRadiusMm});
        // Close the loop by adding the first point of the left curve again for top straight end
        centerPath.push_back({curveRadiusMm * cos(static_cast<float>(M_PI_2)), curveRadiusMm * sin(static_cast<float>(M_PI_2))});

        if (centerPath.size() < 2)
        {
            std::cerr << "TrackDisplay: Center path has too few points." << std::endl;
            return;
        }

        // Generate inner and outer tracks from the center path
        for (size_t i = 0; i < centerPath.size(); ++i)
        {
            sf::Vector2f p1 = centerPath[i];
            sf::Vector2f p2 = centerPath[(i + 1) % centerPath.size()]; // Next point, wrap around for last segment (though path is open now)
            if (i == centerPath.size() - 1)
                p2 = centerPath[0]; // Ensure last segment connects to first if it's a closed loop conceptually

            sf::Vector2f dir = p2 - p1;
            float len = std::sqrt(dir.x * dir.x + dir.y * dir.y);
            if (len < 0.001f)
                continue;                                                  // Avoid division by zero for coincident points
            sf::Vector2f normal = sf::Vector2f(-dir.y / len, dir.x / len); // Perpendicular vector

            // For the first point of each segment
            m_innerTrack.append(sf::Vertex(p1 - normal * trackOffset, m_trackColor));
            m_outerTrack.append(sf::Vertex(p1 + normal * trackOffset, m_trackColor));

            // If it's the last point overall, also add its offset version
            if (i == centerPath.size() - 1)
            {
                m_innerTrack.append(sf::Vertex(p2 - normal * trackOffset, m_trackColor));
                m_outerTrack.append(sf::Vertex(p2 + normal * trackOffset, m_trackColor));
            }
        }
        std::cout << "TrackDisplay: Inner track points: " << m_innerTrack.getVertexCount()
                  << ", Outer track points: " << m_outerTrack.getVertexCount() << std::endl;
    }

private:
    /* // Temporarily disable addArrow
    void addArrow(sf::Vector2f position, float angleDegrees, float headSize) {
        float angleRad = angleDegrees * static_cast<float>(M_PI) / 180.f;
        float cs = cos(angleRad);
        float sn = sin(angleRad);

        sf::Vector2f p1_local(headSize, 0.f);
        sf::Vector2f p2_local(-headSize * 0.5f, headSize * 0.4f);
        sf::Vector2f p3_local(-headSize * 0.5f, -headSize * 0.4f);

        sf::Vector2f p1 = position + sf::Vector2f(p1_local.x * cs - p1_local.y * sn, p1_local.x * sn + p1_local.y * cs);
        sf::Vector2f p2 = position + sf::Vector2f(p2_local.x * cs - p2_local.y * sn, p2_local.x * sn + p2_local.y * cs);
        sf::Vector2f p3 = position + sf::Vector2f(p3_local.x * cs - p3_local.y * sn, p3_local.x * sn + p3_local.y * cs);

        m_directionArrows.append(sf::Vertex(p1, m_arrowColor));
        m_directionArrows.append(sf::Vertex(p2, m_arrowColor));
        m_directionArrows.append(sf::Vertex(p3, m_arrowColor));
    }*/

    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        target.draw(m_innerTrack, states);
        target.draw(m_outerTrack, states);
        // target.draw(m_directionArrows, states); // Arrows temporarily disabled
    }

    float m_trackWidthMm;
    sf::Color m_trackColor;
    sf::Color m_arrowColor;

    sf::VertexArray m_innerTrack;
    sf::VertexArray m_outerTrack;
    // sf::VertexArray m_directionArrows; // Arrows temporarily disabled

    // Unused for now, but kept for consistency with other Display classes if needed later
    float m_mmToPxRatio = 0.1f;
    float m_scaleFactor = 1.0f;
};

// Placeholder for VehicleDisplay class
class VehicleDisplay : public sf::Drawable
{
public:
    VehicleDisplay()
    {
        // Initialize placeholder shape for when texture fails to load
        m_placeholderShape.setRadius(50.f); // Example radius
        m_placeholderShape.setFillColor(sf::Color::Blue);
        m_placeholderShape.setOrigin(m_placeholderShape.getRadius(), m_placeholderShape.getRadius());
    }
    bool loadTexture(const std::string &path)
    {
        if (!m_texture.loadFromFile(path))
        {
            std::cerr << "Error loading vehicle texture: " << path << std::endl;
            return false;
        }
        m_sprite.setTexture(m_texture);
        m_sprite.setOrigin(m_texture.getSize().x / 2.f, m_texture.getSize().y / 2.f);
        return true;
    }
    void setPose(sf::Vector2f positionMm, float angleDeg)
    {
        m_positionMm = positionMm;
        m_angleDeg = angleDeg;
    }
    void setPhysicalSizeMm(sf::Vector2f sizeMm) { m_physicalSizeMm = sizeMm; }
    void setMmToPxRatio(float ratio) { m_mmToPxRatio = ratio; }
    void setScaleFactor(float scale) { m_scaleFactor = scale; } // Overall view scale

private:
    virtual void draw(sf::RenderTarget &target, sf::RenderStates states) const override
    {
        if (!m_texture.getSize().x) // If texture is not loaded or invalid
        {
            // Draw placeholder shape at the vehicle's intended screen position
            float currentScaleFactor = m_mmToPxRatio * m_scaleFactor;
            sf::Vector2f screenPos = sf::Vector2f(m_positionMm.x * currentScaleFactor, m_positionMm.y * currentScaleFactor);

            // Apply position to placeholder, but scale it simply for now
            sf::CircleShape tempPlaceholder = m_placeholderShape; // Make a mutable copy for this draw call
            tempPlaceholder.setPosition(screenPos);
            // Placeholder doesn't use physical size scaling in this simple example
            // but you could scale it based on m_physicalSizeMm if desired.
            target.draw(tempPlaceholder, states);
            return;
        }

        float currentScaleFactor = m_mmToPxRatio * m_scaleFactor;
        sf::Vector2f screenPos = sf::Vector2f(m_positionMm.x * currentScaleFactor, m_positionMm.y * currentScaleFactor);

        float spriteScaleX = (m_physicalSizeMm.x * currentScaleFactor) / m_texture.getSize().x;
        float spriteScaleY = (m_physicalSizeMm.y * currentScaleFactor) / m_texture.getSize().y;

        m_sprite.setPosition(screenPos);
        m_sprite.setRotation(m_angleDeg);
        m_sprite.setScale(spriteScaleX, spriteScaleY);

        target.draw(m_sprite, states);
    }

    sf::Texture m_texture;
    mutable sf::Sprite m_sprite;        // Mutable because draw is const
    sf::CircleShape m_placeholderShape; // For fallback drawing
    sf::Vector2f m_positionMm{0, 0};
    float m_angleDeg = 0.f;
    sf::Vector2f m_physicalSizeMm{2000.f, 1000.f};
    float m_mmToPxRatio = 0.1f;
    float m_scaleFactor = 1.0f;
};

int main()
{
    sf::RenderWindow window(sf::VideoMode(1600, 1200), "Warehouse Simulation View");
    window.setFramerateLimit(60);

    // UI Font and Text for Info Display
    sf::Font uiFont;
    // Attempt to load a common font. Provide a correct path to a .ttf file in your project if this fails.
    // Common paths: "C:/Windows/Fonts/arial.ttf" or a relative path like "resources/fonts/arial.ttf"
    if (!uiFont.loadFromFile("../assets/fonts/arial.ttf"))
    {
        std::cerr << "Error loading ../assets/fonts/arial.ttf. Please ensure a font is available in that path." << std::endl;
        // You might want to fall back to a system font or handle the error more gracefully
        if (!uiFont.loadFromFile("C:/Windows/Fonts/arial.ttf"))
        { // Fallback attempt
            std::cerr << "Error loading C:/Windows/Fonts/arial.ttf as fallback. UI text might not display." << std::endl;
            // return -1; // Optionally exit if font loading fails critically
        }
    }

    sf::Text mouseCoordsText;
    mouseCoordsText.setFont(uiFont);
    mouseCoordsText.setCharacterSize(16); // pixels
    mouseCoordsText.setFillColor(sf::Color::White);
    mouseCoordsText.setPosition(10.f, 10.f);

    sf::Text zoomLevelText;
    zoomLevelText.setFont(uiFont);
    zoomLevelText.setCharacterSize(16);
    zoomLevelText.setFillColor(sf::Color::White);
    zoomLevelText.setPosition(10.f, 30.f);

    // Attempt to initialize TGUI
    // tgui::GuiSFML gui(window); // Use tgui::GuiSFML for SFML specific backend

    // try
    // {
    //     // Add a TGUI Label for testing
    //     auto testLabel = tgui::Label::create("TGUI Initialized!");
    //     testLabel->setPosition(10, 10);
    //     testLabel->getRenderer()->setTextColor(sf::Color::White);
    //     gui.add(testLabel);
    // }
    // catch (const tgui::Exception &e)
    // {
    //     std::cerr << "TGUI Exception: " << e.what() << std::endl;
    //     return -1;
    // }
    // catch (const std::exception &e)
    // {
    //     std::cerr << "Standard Exception during TGUI init: " << e.what() << std::endl;
    //     return -1;
    // }
    // catch (...)
    // {
    //     std::cerr << "Unknown exception during TGUI init." << std::endl;
    //     return -1;
    // }

    // TODO: Instantiate TrackDisplay, WarehouseDisplay, VehicleDisplay
    // TODO: Setup view, mmToPxRatio, scaleFactor, etc.

    sf::View simView = window.getDefaultView();
    simView.setCenter(0.f, 0.f);                                // Explicitly center the initial view on world origin (0,0)
    float currentZoomFactor = 1.0f;                             // Tracks the current zoom level of simView
    const sf::Vector2f initialViewCenter = simView.getCenter(); // This will now be (0.f, 0.f)
    const float initialZoomFactor = 1.0f;                       // Assuming default view starts at 1.0f zoom relative to this new center

    std::cout << "SimulationView started. UI text should be in English." << std::endl;
    std::cout << "Controls: LMB Drag to Pan, Mouse Wheel to Zoom, Space to Reset View." << std::endl;

    // Instantiate display objects
    TrackDisplay trackDisplay;
    trackDisplay.setTrackWidthMm(1200.f); // Example width
    // trackDisplay.setTrackColor(sf::Color(80, 80, 80)); // Optional: set different color
    trackDisplay.generateTrack(40000.f, 2500.f); // Actual dimensions from doc

    WarehouseDisplay warehouseDisplay(uiFont);
    // Add a few example warehouses
    warehouseDisplay.addWarehouse(13, {200.f, 200.f}, {100.f, 50.f}, false); // ID, Pos, Size, isOutbound
    warehouseDisplay.addWarehouse(14, {400.f, 200.f}, {100.f, 50.f}, true);
    warehouseDisplay.addWarehouse(1, {-200.f, -50.f}, {80.f, 120.f}, false);

    VehicleDisplay vehicleDisplay;
    if (!vehicleDisplay.loadTexture("../assets/image/wuliu-car.jpg"))
    {
        std::cerr << "Failed to load vehicle texture for VehicleDisplay." << std::endl;
    }
    vehicleDisplay.setPhysicalSizeMm({2000.f, 1000.f}); // Length, Width from docs
    vehicleDisplay.setMmToPxRatio(0.05f);               // Let's try a smaller ratio for the car to make it reasonably sized initially
    vehicleDisplay.setScaleFactor(1.0f);                // Initial view scale factor for the object itself
    vehicleDisplay.setPose({-200.f, -200.f}, 0.f);      // Position it somewhere visible

    sf::Vector2f mouseWorldPos{0.f, 0.f};
    bool isPanning = false;
    sf::Vector2i lastMousePos;

    while (window.isOpen())
    {
        sf::Vector2i mouseWindowPos = sf::Mouse::getPosition(window);
        mouseWorldPos = window.mapPixelToCoords(mouseWindowPos, simView);

        sf::Event event;
        while (window.pollEvent(event))
        {
            // gui.handleEvent(event); // Pass events to TGUI

            if (event.type == sf::Event::Closed)
            {
                window.close();
            }

            // View Panning
            if (event.type == sf::Event::MouseButtonPressed)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    isPanning = true;
                    lastMousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                }
            }
            if (event.type == sf::Event::MouseButtonReleased)
            {
                if (event.mouseButton.button == sf::Mouse::Left)
                {
                    isPanning = false;
                }
            }
            if (event.type == sf::Event::MouseMoved)
            {
                if (isPanning)
                {
                    sf::Vector2i newMousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
                    sf::Vector2f delta = window.mapPixelToCoords(lastMousePos, simView) - window.mapPixelToCoords(newMousePos, simView);
                    simView.move(delta);
                    lastMousePos = newMousePos;
                }
            }

            // View Zooming
            if (event.type == sf::Event::MouseWheelScrolled)
            {
                if (event.mouseWheelScroll.delta > 0) // Scroll up, zoom in
                {
                    simView.zoom(0.9f);
                    currentZoomFactor *= 0.9f;
                }
                else if (event.mouseWheelScroll.delta < 0) // Scroll down, zoom out
                {
                    simView.zoom(1.1f);
                    currentZoomFactor *= 1.1f;
                }
            }

            // View Reset
            if (event.type == sf::Event::KeyPressed)
            {
                if (event.key.code == sf::Keyboard::Space) // Reset view with Space bar
                {
                    simView.setCenter(initialViewCenter);
                    simView.setSize(window.getDefaultView().getSize()); // Reset size/zoom to original default
                    // Adjust zoom level of simView to match initialZoomFactor relative to default size
                    simView.zoom(initialZoomFactor / currentZoomFactor); // this line is tricky, adjust if default zoom isn't 1.0f
                    currentZoomFactor = initialZoomFactor;
                }
            }

            // if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::T)
            // {
            //     show_label = !show_label;
            //     gui.get<tgui::Label>("TestLabel")->setVisible(show_label); // Assuming we give it an ID
            // }
        }

        window.clear(sf::Color(50, 50, 50)); // Darker background

        window.setView(simView); // Set view for simulation objects
        // TODO: Draw TrackDisplay, WarehouseDisplay, VehicleDisplay
        window.draw(trackDisplay);
        window.draw(warehouseDisplay);
        window.draw(vehicleDisplay);

        // Example placeholder drawing:
        // sf::CircleShape placeholderCircle(50.f);

        // --- Draw UI elements ---
        window.setView(window.getDefaultView()); // Reset view for GUI elements

        std::ostringstream mousePosStream;
        mousePosStream << "Mouse (mm): (" << static_cast<int>(mouseWorldPos.x) << ", " << static_cast<int>(mouseWorldPos.y) << ")";
        mouseCoordsText.setString(mousePosStream.str());

        std::ostringstream zoomStream;
        zoomStream.precision(2);
        zoomStream << std::fixed << "Zoom: " << (1.0f / currentZoomFactor * 100.f) << "%";
        zoomLevelText.setString(zoomStream.str());

        window.draw(mouseCoordsText);
        window.draw(zoomLevelText);
        // gui.draw(); // Draw TGUI elements

        window.display();
    }

    return 0;
}