#include "SimulationView.hpp"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>

/**
 * @brief 构造函数
 * 
 * 初始化渲染器和视图参数
 */
SimulationView::SimulationView()
{
    // 尝试加载字体
    tryLoadFont();
    
    // 设置信息面板
    m_infoPanel.setSize(sf::Vector2f(300.0f, 150.0f));
    m_infoPanel.setFillColor(sf::Color(30, 30, 30, 200));
    m_infoPanel.setOutlineColor(sf::Color(100, 100, 100));
    m_infoPanel.setOutlineThickness(1.0f);
    
    if (m_fontLoaded) {
        m_infoPanelText.setFont(m_font);
        m_infoPanelText.setCharacterSize(14);
        m_infoPanelText.setFillColor(sf::Color::White);
    }
    
    // 设置轨道参数
    m_trackRenderer.generateGeometry(126000.0f, 2500.0f);
}

/**
 * @brief 设置仿真接口
 * @param interface 仿真接口对象
 */
void SimulationView::setSimulationInterface(std::shared_ptr<SimulationInterface> interface)
{
    m_interface = interface;
    
    if (m_interface) {
        // 设置轨道参数
        m_trackRenderer.generateGeometry(
            m_interface->getTrackLength(),
            m_interface->getCurveRadius()
        );
        
        // 设置回调函数，当车辆或仓库状态更新时更新渲染器
        m_interface->setVehicleUpdateCallback([this](const std::vector<VehicleState>& vehicles) {
            m_vehicleRenderer.updateVehicles(vehicles);
        });
        
        m_interface->setWarehouseUpdateCallback([this](const std::vector<WarehouseState>& warehouses) {
            m_warehouseRenderer.updateWarehouses(warehouses);
        });
        
        // 初始获取数据
        m_vehicleRenderer.updateVehicles(m_interface->getVehicleStates());
        m_warehouseRenderer.updateWarehouses(m_interface->getWarehouseStates());
    }
    
    // 如果有加载字体，传递给渲染器
    if (m_fontLoaded) {
        m_vehicleRenderer.setFont(m_font);
        m_warehouseRenderer.setFont(m_font);
    }
}

/**
 * @brief 调整视图大小
 * @param width 窗口宽度
 * @param height 窗口高度
 */
void SimulationView::resize(unsigned int width, unsigned int height)
{
    // 重置视图大小
    m_view.reset(sf::FloatRect(0, 0, static_cast<float>(width), static_cast<float>(height)));
    m_view.zoom(1.0f / m_zoomLevel); // 应用当前缩放级别
    
    // 重置视图中心
    m_view.setCenter(m_viewCenter);
}

/**
 * @brief 处理鼠标按下事件
 * @param mousePos 鼠标位置（窗口坐标）
 * @param button 按下的按钮
 */
void SimulationView::handleMouseButtonPressed(const sf::Vector2i& mousePos, sf::Mouse::Button button)
{
    if (button == sf::Mouse::Left) {
        // 开始拖动视图
        m_isPanning = true;
        m_lastMousePos = mousePos;
    } else if (button == sf::Mouse::Right) {
        // 选择对象
        sf::Vector2f worldPos = sf::Vector2f(
            m_view.getCenter().x + (mousePos.x - m_view.getSize().x / 2) / m_zoomLevel,
            m_view.getCenter().y + (mousePos.y - m_view.getSize().y / 2) / m_zoomLevel
        );
        selectObjectAt(worldPos);
    }
}

/**
 * @brief 处理鼠标释放事件
 * @param mousePos 鼠标位置（窗口坐标）
 * @param button 释放的按钮
 */
void SimulationView::handleMouseButtonReleased(const sf::Vector2i& mousePos, sf::Mouse::Button button)
{
    if (button == sf::Mouse::Left) {
        // 停止拖动视图
        m_isPanning = false;
    }
}

/**
 * @brief 处理鼠标移动事件
 * @param mousePos 鼠标位置（窗口坐标）
 */
void SimulationView::handleMouseMoved(const sf::Vector2i& mousePos)
{
    if (m_isPanning) {
        // 计算鼠标移动距离
        sf::Vector2f delta(
            static_cast<float>(m_lastMousePos.x - mousePos.x) / m_zoomLevel,
            static_cast<float>(m_lastMousePos.y - mousePos.y) / m_zoomLevel
        );
        
        // 更新视图中心
        m_viewCenter += delta;
        m_view.setCenter(m_viewCenter);
        
        // 更新鼠标位置
        m_lastMousePos = mousePos;
    }
}

/**
 * @brief 处理鼠标滚轮事件
 * @param delta 滚轮增量
 * @param mousePos 鼠标位置（窗口坐标）
 */
void SimulationView::handleMouseWheelScrolled(float delta, const sf::Vector2i& mousePos)
{
    // 计算缩放因子
    float factor = (delta > 0) ? 1.1f : 0.9f;
    
    // 更新缩放级别
    m_zoomLevel *= factor;
    
    // 限制缩放范围
    if (m_zoomLevel < 0.2f) m_zoomLevel = 0.2f;
    if (m_zoomLevel > 5.0f) m_zoomLevel = 5.0f;
    
    // 应用缩放
    m_view.setSize(m_view.getSize().x / factor, m_view.getSize().y / factor);
}

/**
 * @brief 处理键盘事件
 * @param key 键代码
 */
void SimulationView::handleKeyPressed(sf::Keyboard::Key key)
{
    switch (key)
    {
        case sf::Keyboard::R:
            // 重置视图
            centerView();
            break;
            
        case sf::Keyboard::Space:
            // 切换暂停/继续
            if (m_interface) {
                m_interface->setPaused(!m_interface->isPaused());
            }
            break;
            
        case sf::Keyboard::Add:
        case sf::Keyboard::Equal:
            // 加快仿真速度
            if (m_interface) {
                float speed = m_interface->getSimulationSpeed();
                m_interface->setSimulationSpeed(speed * 1.2f);
            }
            break;
            
        case sf::Keyboard::Subtract:
        case sf::Keyboard::Dash:
            // 减慢仿真速度
            if (m_interface) {
                float speed = m_interface->getSimulationSpeed();
                m_interface->setSimulationSpeed(speed / 1.2f);
            }
            break;
            
        case sf::Keyboard::I:
            // 显示/隐藏信息面板
            m_showInfoPanel = !m_showInfoPanel;
            break;
            
        default:
            break;
    }
}

/**
 * @brief 更新逻辑
 * @param deltaTime 时间增量（秒）
 */
void SimulationView::update(float deltaTime)
{
    // 更新信息面板
    if (m_showInfoPanel) {
        updateInfoPanel();
    }
}

/**
 * @brief 将视图居中到轨道中央
 */
void SimulationView::centerView()
{
    // 重置视图参数
    m_viewCenter = sf::Vector2f(0.0f, 0.0f);
    m_zoomLevel = 0.8f;
    
    // 应用视图参数
    m_view.setCenter(m_viewCenter);
    m_view.setSize(m_view.getSize() * (1.0f / m_zoomLevel));
}

/**
 * @brief 重载的绘制方法
 * @param target 渲染目标
 * @param states 渲染状态
 */
void SimulationView::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    // 绘制轨道
    target.draw(m_trackRenderer, states);
    
    // 绘制仓库/接口设备
    target.draw(m_warehouseRenderer, states);
    
    // 绘制车辆
    target.draw(m_vehicleRenderer, states);
    
    // 绘制信息面板
    if (m_showInfoPanel) {
        // 重置视图为默认，以便UI元素不受世界坐标影响
        sf::View defaultView = target.getDefaultView();
        target.setView(defaultView);
        
        // 绘制信息面板背景
        target.draw(m_infoPanel);
        
        // 绘制信息面板文本
        if (m_fontLoaded) {
            target.draw(m_infoPanelText);
        }
        
        // 恢复原视图
        target.setView(m_view);
    }
}

/**
 * @brief 选择视图中的对象
 * @param worldPos 世界坐标
 * 
 * 检测点击位置是否有车辆或仓库/接口设备，并选中最近的一个
 */
void SimulationView::selectObjectAt(const sf::Vector2f& worldPos)
{
    if (!m_interface) return;
    
    // 获取车辆和仓库数据
    std::vector<VehicleState> vehicles = m_interface->getVehicleStates();
    std::vector<WarehouseState> warehouses = m_interface->getWarehouseStates();
    
    // 轨道参数
    float trackLength = m_interface->getTrackLength();
    float straightLength = 40000.0f; // 假设值，应从接口获取
    float curveRadius = m_interface->getCurveRadius();
    
    // 最近对象的距离和ID
    float minDistance = 100.0f; // 点击误差范围（像素）
    int nearestId = -1;
    bool isVehicle = false;
    
    // 检查是否点击了车辆
    for (const auto& vehicle : vehicles) {
        // 计算车辆世界坐标
        sf::Vector2f vehiclePos;
        float rotation;
        m_vehicleRenderer.calculateWorldPositionAndRotation(
            vehicle.trackPosition,
            trackLength,
            straightLength,
            curveRadius,
            vehiclePos,
            rotation
        );
        
        // 计算点击点到车辆的距离
        float distance = std::sqrt(
            (vehiclePos.x - worldPos.x) * (vehiclePos.x - worldPos.x) +
            (vehiclePos.y - worldPos.y) * (vehiclePos.y - worldPos.y)
        );
        
        // 如果距离在车辆尺寸范围内，且比当前最近的还近，则选中该车辆
        sf::Vector2f vehicleSize = m_vehicleRenderer.getVehicleSize();
        float vehicleRadius = std::max(vehicleSize.x, vehicleSize.y) / 2.0f;
        
        if (distance < vehicleRadius && distance < minDistance) {
            minDistance = distance;
            nearestId = vehicle.id;
            isVehicle = true;
        }
    }
    
    // 如果没有找到车辆，检查是否点击了仓库/接口设备
    if (nearestId < 0) {
        for (const auto& warehouse : warehouses) {
            // 计算仓库世界坐标
            sf::Vector2f warehousePos = m_warehouseRenderer.calculateWorldPosition(
                warehouse.trackPosition,
                trackLength,
                straightLength,
                curveRadius
            );
            
            // 计算点击点到仓库的距离
            float distance = std::sqrt(
                (warehousePos.x - worldPos.x) * (warehousePos.x - worldPos.x) +
                (warehousePos.y - worldPos.y) * (warehousePos.y - worldPos.y)
            );
            
            // 如果距离在仓库尺寸范围内，且比当前最近的还近，则选中该仓库
            sf::Vector2f warehouseSize = m_warehouseRenderer.getSize(warehouse.isInterface);
            float warehouseRadius = std::max(warehouseSize.x, warehouseSize.y) / 2.0f;
            
            if (distance < warehouseRadius && distance < minDistance) {
                minDistance = distance;
                nearestId = warehouse.id;
                isVehicle = false;
            }
        }
    }
    
    // 更新选中对象
    m_selectedObjectId = nearestId;
    m_selectedIsVehicle = isVehicle;
    
    // 如果选中了对象，显示信息面板
    if (m_selectedObjectId >= 0) {
        m_showInfoPanel = true;
        updateInfoPanel();
    }
}

/**
 * @brief 更新信息面板
 * 
 * 根据当前选中的对象，更新信息面板的内容
 */
void SimulationView::updateInfoPanel()
{
    if (!m_interface || !m_fontLoaded) return;
    
    std::ostringstream info;
    
    // 如果有选中对象
    if (m_selectedObjectId >= 0) {
        if (m_selectedIsVehicle) {
            // 获取车辆信息
            const auto& vehicles = m_interface->getVehicleStates();
            for (const auto& vehicle : vehicles) {
                if (vehicle.id == m_selectedObjectId) {
                    info << "车辆 ID: " << vehicle.id << "\n";
                    info << "位置: " << std::fixed << std::setprecision(2) << vehicle.trackPosition << " mm\n";
                    info << "速度: " << std::fixed << std::setprecision(2) << vehicle.speed << " mm/s\n";
                    info << "状态: " << (vehicle.isLoaded ? "载货中" : "空载") << "\n";
                    info << "任务: " << (vehicle.currentTaskId >= 0 ? std::to_string(vehicle.currentTaskId) : "无") << "\n";
                    break;
                }
            }
        } else {
            // 获取仓库信息
            const auto& warehouses = m_interface->getWarehouseStates();
            for (const auto& warehouse : warehouses) {
                if (warehouse.id == m_selectedObjectId) {
                    info << (warehouse.isInterface ? "接口设备 ID: " : "仓库 ID: ") << warehouse.id << "\n";
                    info << "位置: " << std::fixed << std::setprecision(2) << warehouse.trackPosition << " mm\n";
                    info << "容量: " << warehouse.capacity << "\n";
                    info << "当前负载: " << warehouse.currentLoad << "\n";
                    info << "负载率: " << std::fixed << std::setprecision(1) 
                         << (warehouse.capacity > 0 ? warehouse.currentLoad * 100.0f / warehouse.capacity : 0.0f) << "%\n";
                    break;
                }
            }
        }
    }
    
    // 添加仿真状态信息
    info << "\n[仿真状态]\n";
    info << "速度: " << std::fixed << std::setprecision(1) << m_interface->getSimulationSpeed() << "x\n";
    info << "状态: " << (m_interface->isPaused() ? "已暂停" : "运行中") << "\n";
    
    // 更新文本
    m_infoPanelText.setString(info.str());
    
    // 调整文本位置
    m_infoPanel.setPosition(10.0f, 10.0f);
    m_infoPanelText.setPosition(20.0f, 15.0f);
}

/**
 * @brief 尝试加载字体
 * @return 是否成功加载
 */
bool SimulationView::tryLoadFont()
{
    // 尝试多个可能的路径
    std::vector<std::string> fontPaths = {
        "resources/fonts/arial.ttf",
        "GUI/resources/fonts/arial.ttf",
        "fonts/arial.ttf",
        "C:/Windows/Fonts/arial.ttf",
        "C:/Windows/Fonts/simhei.ttf" // 添加中文字体支持
    };
    
    for (const auto& path : fontPaths) {
        try {
            if (m_font.loadFromFile(path)) {
                m_fontLoaded = true;
                std::cout << "成功加载字体: " << path << std::endl;
                return true;
            }
        } catch (const std::exception& e) {
            std::cerr << "尝试加载字体失败: " << path << " - " << e.what() << std::endl;
        }
    }
    
    std::cerr << "无法加载任何字体，将使用默认显示" << std::endl;
    return false;
}
