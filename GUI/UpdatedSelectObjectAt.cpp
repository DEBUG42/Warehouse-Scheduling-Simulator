void SimulationView::selectObjectAt(const sf::Vector2f &worldPos)
{
    // 实现对象选择逻辑
    // 首先检查是否点击了车辆
    const std::vector<VehicleState>& vehicles = m_engine->getVehicles();
    for (const auto& vehicle : vehicles)
    {
        // 计算车辆位置
        float angle = vehicle.position * 2.0f * M_PI;
        float radius = m_engine->getTrackRadius();
        float x = radius * std::cos(angle);
        float y = radius * std::sin(angle) * 0.7f;
        
        sf::Vector2f vehiclePos(x, y);
        float distance = std::sqrt(std::pow(vehiclePos.x - worldPos.x, 2) +
                                  std::pow(vehiclePos.y - worldPos.y, 2));

        // 如果点击位置在车辆半径内（假设半径为10个单位）
        if (distance < 10.0f)
        {
            // 创建车辆模拟对象
            m_selectedObject = std::make_shared<SimObject>(
                SimObject::ObjectType::Vehicle,
                vehicle.id);
            return;
        }
    }

    // 然后检查是否点击了设备
    for (const auto &devicePair : m_engine->getDevices())
    {
        int deviceId = devicePair.first;
        sf::Vector2f devicePos = m_engine->getDevicePosition(deviceId);
        float distance = std::sqrt(std::pow(devicePos.x - worldPos.x, 2) +
                                  std::pow(devicePos.y - worldPos.y, 2));

        // 如果点击位置在设备半径内（假设半径为15个单位）
        if (distance < 15.0f)
        {
            // 创建设备模拟对象
            m_selectedObject = std::make_shared<SimObject>(
                SimObject::ObjectType::Device,
                deviceId);
            return;
        }
    }

    // 如果点击的是空白区域，取消选择
    m_selectedObject = nullptr;
}
