#pragma once
#include <string>
#include <fstream>
#include <iostream>

// 简单的JSON配置读取类
class ConfigReader {
private:
    std::string m_configPath;
    // 存储配置参数
    struct {
        // 仿真配置
        float trackLength = 200.0f;
        float curveRadius = 50.0f;
        float initialTimeScale = 1.0f;
        float maxTimeScale = 5.0f;
        int vehicleCount = 5;
        
        // 显示配置
        int windowWidth = 1280;
        int windowHeight = 720;
        float toolbarHeight = 30.0f;
        float statusPanelWidth = 300.0f;
        int framerate = 60;
        
        // 设备配置
        int storageInCount = 6;
        int storageOutCount = 6;
        int workstationInCount = 3;
        int workstationOutCount = 3;
        
        // 资源路径
        std::string fontPath = "resources/fonts/arial.ttf";
        std::string iconBasePath = "resources/icons/";
    } m_config;
    
public:
    ConfigReader(const std::string& configPath = "config.json") 
        : m_configPath(configPath) 
    {}
    
    bool loadConfig() {
        // 实际应用中应使用完整的JSON解析库
        // 这里使用简化版本，仅读取预定义键值
        std::ifstream file(m_configPath);
        if (!file.is_open()) {
            std::cerr << "无法打开配置文件: " << m_configPath << std::endl;
            return false;
        }
        
        std::string line;
        
        // 解析配置文件内容 (简化实现)
        while (std::getline(file, line)) {
            auto readValue = [&line](const std::string& key, auto& value, auto defaultValue) {
                size_t pos = line.find(key);
                if (pos != std::string::npos) {
                    size_t valueStart = line.find(":", pos + key.length());
                    if (valueStart != std::string::npos) {
                        size_t contentStart = line.find_first_not_of(" \t", valueStart + 1);
                        if (contentStart != std::string::npos) {
                            // 查找数值结束位置
                            size_t contentEnd;
                            if (std::is_same<decltype(value), std::string>::value) {
                                // 字符串类型 (在引号内)
                                contentStart = line.find("\"", contentStart) + 1;
                                contentEnd = line.find("\"", contentStart);
                            } else {
                                // 数值类型
                                contentEnd = line.find_first_of(",}", contentStart);
                            }
                            
                            if (contentEnd != std::string::npos) {
                                std::string valueStr = line.substr(contentStart, contentEnd - contentStart);
                                try {
                                    if constexpr (std::is_same<decltype(value), int>::value) {
                                        value = std::stoi(valueStr);
                                    } else if constexpr (std::is_same<decltype(value), float>::value) {
                                        value = std::stof(valueStr);
                                    } else if constexpr (std::is_same<decltype(value), std::string>::value) {
                                        value = valueStr;
                                    }
                                } catch (...) {
                                    value = defaultValue;
                                }
                                return true;
                            }
                        }
                    }
                }
                return false;
            };
            
            // 仿真配置
            readValue("\"trackLength\"", m_config.trackLength, 200.0f);
            readValue("\"curveRadius\"", m_config.curveRadius, 50.0f);
            readValue("\"initialTimeScale\"", m_config.initialTimeScale, 1.0f);
            readValue("\"maxTimeScale\"", m_config.maxTimeScale, 5.0f);
            readValue("\"vehicleCount\"", m_config.vehicleCount, 5);
            
            // 显示配置
            readValue("\"windowWidth\"", m_config.windowWidth, 1280);
            readValue("\"windowHeight\"", m_config.windowHeight, 720);
            readValue("\"toolbarHeight\"", m_config.toolbarHeight, 30.0f);
            readValue("\"statusPanelWidth\"", m_config.statusPanelWidth, 300.0f);
            readValue("\"framerate\"", m_config.framerate, 60);
            
            // 设备配置
            readValue("\"storageInCount\"", m_config.storageInCount, 6);
            readValue("\"storageOutCount\"", m_config.storageOutCount, 6);
            readValue("\"workstationInCount\"", m_config.workstationInCount, 3);
            readValue("\"workstationOutCount\"", m_config.workstationOutCount, 3);
            
            // 资源路径
            readValue("\"fontPath\"", m_config.fontPath, std::string("resources/fonts/arial.ttf"));
            readValue("\"iconBasePath\"", m_config.iconBasePath, std::string("resources/icons/"));
        }
        
        file.close();
        return true;
    }
    
    // 获取配置参数
    float getTrackLength() const { return m_config.trackLength; }
    float getCurveRadius() const { return m_config.curveRadius; }
    float getInitialTimeScale() const { return m_config.initialTimeScale; }
    float getMaxTimeScale() const { return m_config.maxTimeScale; }
    int getVehicleCount() const { return m_config.vehicleCount; }
    
    int getWindowWidth() const { return m_config.windowWidth; }
    int getWindowHeight() const { return m_config.windowHeight; }
    float getToolbarHeight() const { return m_config.toolbarHeight; }
    float getStatusPanelWidth() const { return m_config.statusPanelWidth; }
    int getFramerate() const { return m_config.framerate; }
    
    int getStorageInCount() const { return m_config.storageInCount; }
    int getStorageOutCount() const { return m_config.storageOutCount; }
    int getWorkstationInCount() const { return m_config.workstationInCount; }
    int getWorkstationOutCount() const { return m_config.workstationOutCount; }
    
    std::string getFontPath() const { return m_config.fontPath; }
    std::string getIconBasePath() const { return m_config.iconBasePath; }
};
