#pragma once

#include <SFML/Graphics.hpp>
#include <TGUI/TGUI.hpp>
#include <memory>
#include <string>
#include <functional>

namespace WarehouseUI
{

    /**
     * @brief TGUIWrapper - 对TGUI库的简单封装，方便集成到仓储仿真系统中
     *
     * 此类主要功能：
     * 1. 统一管理GUI资源和主题
     * 2. 提供简化的界面创建API
     * 3. 处理事件和绘制
     */
    class TGUIWrapper
    {
    public:
        /**
         * @brief 构造函数
         * @param window SFML渲染窗口
         * @param themePath 主题文件路径
         */
        TGUIWrapper(sf::RenderWindow &window, const std::string &themePath = "")
        {
            m_gui = std::make_unique<tgui::Gui>(window);

            if (!themePath.empty())
            {
                try
                {
                    m_theme = std::make_unique<tgui::Theme>(themePath);
                }
                catch (const tgui::Exception &e)
                {
                    std::cerr << "无法加载主题: " << e.what() << std::endl;
                }
            }
        }

        /**
         * @brief 处理事件
         * @param event SFML事件
         */
        void handleEvent(sf::Event event)
        {
            m_gui->handleEvent(event);
        }

        /**
         * @brief 绘制GUI
         */
        void draw()
        {
            m_gui->draw();
        }

        /**
         * @brief 创建按钮
         * @param text 按钮文本
         * @param position 位置
         * @param size 大小
         * @param callback 点击回调函数
         * @return 按钮指针
         */
        tgui::Button::Ptr createButton(const std::string &text,
                                       const tgui::Layout2d &position,
                                       const tgui::Layout2d &size,
                                       const std::function<void()> &callback = nullptr)
        {
            tgui::Button::Ptr button;

            if (m_theme)
            {
                button = m_theme->load("Button");
            }
            else
            {
                button = tgui::Button::create();
            }

            button->setText(text);
            button->setPosition(position);
            button->setSize(size);

            if (callback)
            {
                button->onPress(callback);
            }

            m_gui->add(button);
            return button;
        }

        /**
         * @brief 创建标签
         * @param text 标签文本
         * @param position 位置
         * @param textSize 文本大小
         * @return 标签指针
         */
        tgui::Label::Ptr createLabel(const std::string &text,
                                     const tgui::Layout2d &position,
                                     unsigned int textSize = 14)
        {
            tgui::Label::Ptr label;

            if (m_theme)
            {
                label = m_theme->load("Label");
            }
            else
            {
                label = tgui::Label::create();
            }

            label->setText(text);
            label->setPosition(position);
            label->setTextSize(textSize);

            m_gui->add(label);
            return label;
        }

        /**
         * @brief 创建面板
         * @param position 位置
         * @param size 大小
         * @return 面板指针
         */
        tgui::Panel::Ptr createPanel(const tgui::Layout2d &position,
                                     const tgui::Layout2d &size)
        {
            tgui::Panel::Ptr panel;

            if (m_theme)
            {
                panel = m_theme->load("Panel");
            }
            else
            {
                panel = tgui::Panel::create();
            }

            panel->setPosition(position);
            panel->setSize(size);

            m_gui->add(panel);
            return panel;
        }

        /**
         * @brief 创建下拉列表
         * @param position 位置
         * @param size 大小
         * @param callback 选择回调函数
         * @return 下拉列表指针
         */
        tgui::ComboBox::Ptr createComboBox(const tgui::Layout2d &position,
                                           const tgui::Layout2d &size,
                                           const std::function<void(std::string)> &callback = nullptr)
        {
            tgui::ComboBox::Ptr comboBox;

            if (m_theme)
            {
                comboBox = m_theme->load("ComboBox");
            }
            else
            {
                comboBox = tgui::ComboBox::create();
            }

            comboBox->setPosition(position);
            comboBox->setSize(size);

            if (callback)
            {
                comboBox->onItemSelect([callback](std::string item)
                                       { callback(item); });
            }

            m_gui->add(comboBox);
            return comboBox;
        }

        /**
         * @brief 获取GUI对象
         * @return GUI对象
         */
        tgui::Gui *getGui()
        {
            return m_gui.get();
        }

        /**
         * @brief 从GUI中移除控件
         * @param widget 控件指针
         */
        void remove(tgui::Widget::Ptr widget)
        {
            m_gui->remove(widget);
        }

        /**
         * @brief 移除所有控件
         */
        void removeAllWidgets()
        {
            m_gui->removeAllWidgets();
        }

    private:
        std::unique_ptr<tgui::Gui> m_gui;
        std::unique_ptr<tgui::Theme> m_theme;
    };

} // namespace WarehouseUI