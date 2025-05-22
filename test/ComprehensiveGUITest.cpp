#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <string>
#include <memory>
#include <sstream>
#include <iomanip>

// GUI component header files
#include "gui/UIControls.hpp"
#include "gui/Toolbar.hpp"
#include "gui/StatusPanel.hpp"
#include "gui/TaskListView.hpp"
#include "gui/ObjectInspector.hpp"
#include "gui/VehicleRenderer.hpp"
#include "gui/MainWindow.hpp"

// Data structure header files
#include "gui/SimObject.hpp"
#include "gui/DeviceState.hpp"
#include "Core/Task.hpp"
#include "gui/MockSimulationInterface.hpp"

// Define test window size
const unsigned int WINDOW_WIDTH = 1280;
const unsigned int WINDOW_HEIGHT = 720;

// Reference TrackRendererGUITest.cpp for successful font path
const std::string FONT_PATH = "../assets/fonts/arial.ttf"; // Assume this is the path used in TrackRendererGUITest

// Helper function: draw a simple device shape
void drawSimpleDevice(sf::RenderTarget &target, const gui::DeviceState &device, sf::Font &font, const sf::Color &color)
{
    float deviceWidthPx = 60.f;
    float deviceHeightPx = 40.f;
    sf::RectangleShape shape(sf::Vector2f(deviceWidthPx, deviceHeightPx));
    shape.setOrigin(deviceWidthPx / 2.f, deviceHeightPx / 2.f);
    shape.setPosition(device.getPosition());
    shape.setFillColor(color);
    shape.setOutlineThickness(2.f);
    shape.setOutlineColor(sf::Color(color.r / 2, color.g / 2, color.b / 2));
    target.draw(shape);

    // Draw device ID
    sf::Text idText(device.getId(), font, 10);
    sf::FloatRect textBounds = idText.getLocalBounds();
    idText.setOrigin(textBounds.left + textBounds.width / 2.0f, textBounds.top + textBounds.height / 2.0f);
    idText.setPosition(device.getPosition());
    idText.setFillColor(sf::Color::White);
    target.draw(idText);
}

int main()
{
    // 创建模拟仿真接口，初始化任务、车辆等状态
    std::shared_ptr<SimulationInterface> simInterface = std::make_shared<MockSimulationInterface>(5);
    MainWindow mainWindow;
    mainWindow.initialize(simInterface);
    mainWindow.runEventLoop();
    return 0;
}