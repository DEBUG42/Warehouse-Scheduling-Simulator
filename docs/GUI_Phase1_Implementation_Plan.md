# GUI 优化第一阶段实施计划

## 目标概述

完成最关键的用户体验改善，让界面更直观、更美观。

## 具体任务清单

### 1. Toolbar 时间显示优化 ⏰

**文件：** `src/gui/Toolbar.cpp`, `include/gui/UIControls.hpp`

**当前状态：**

```cpp
// 显示两个时间：仿真时间 + 实时时间
m_simTimeText.setString("Sim: " + std::to_string(simTime));
m_realTimeText.setString("Real: " + std::to_string(realTime));
```

**优化目标：**

```cpp
// 仅显示格式化的仿真时间
std::string formatTime(float seconds) {
    int hours = (int)(seconds / 3600);
    int minutes = (int)((seconds - hours * 3600) / 60);
    int secs = (int)(seconds) % 60;
    int millis = (int)((seconds - (int)seconds) * 1000);
    return string_format("%02d:%02d:%02d.%03d", hours, minutes, secs, millis);
}
```

**实施步骤：**

1. 修改 TimeDisplay 类，移除 realTime 相关代码
2. 添加 formatTime 工具函数
3. 更新时间显示布局，留出更多空间给其他控件

---

### 2. StatusPanel 比例调整 📐

**文件：** `src/gui/StatusPanel.cpp`

**当前比例：**

- 全局状态区：~20%
- 对象检视器：35%
- 任务列表：~45%

**优化比例：**

- 全局状态区：25%（增加更多系统信息）
- 对象检视器：35%（保持）
- 任务列表：40%（略减少，为全局状态让空间）

**实施步骤：**

1. 调整 StatusPanel.cpp 中的区域高度计算
2. 增强分隔线视觉效果（阴影+颜色）
3. 测试不同窗口尺寸下的显示效果

---

### 3. TaskListView 视觉优化 🎨

**文件：** `src/gui/TaskListView.cpp`, `include/gui/TaskListView.hpp`

**当前显示：**

```
Task ID: T_001, Type: 1, Status: 0
Task ID: T_002, Type: 2, Status: 1
...
```

**优化显示：**

```
🔴 [高优先级] T_001: 入库任务 A1→B2 (进行中)
🟡 [中优先级] T_002: 出库任务 C3→出口 (等待中)
🟢 [低优先级] T_003: 移库任务 D1→D2 (排队中)
```

**实施步骤：**

1. 添加任务优先级颜色映射函数
2. 改进任务信息格式化函数
3. 实现鼠标悬停高亮效果
4. 添加任务类型图标（可选，用 Unicode 符号）

---

## 第一阶段代码修改预览

### 1. 时间格式化工具函数

```cpp
// 在 include/gui/UIControls.hpp 中添加
namespace UIUtils {
    std::string formatSimulationTime(float seconds);
    sf::Color getPriorityColor(int priority);
    std::string getTaskTypeIcon(int taskType);
}
```

### 2. StatusPanel 比例常量

```cpp
// 在 StatusPanel.cpp 中修改
const float GLOBAL_STATUS_HEIGHT_RATIO = 0.25f;  // 25%
const float OBJECT_INSPECTOR_HEIGHT_RATIO = 0.35f; // 35%
const float TASK_LIST_HEIGHT_RATIO = 0.40f;      // 40%
```

### 3. 任务显示格式化

```cpp
// 在 TaskListView.cpp 中添加
std::string formatTaskDisplay(const Core::Task& task) {
    std::string priorityIcon = getPriorityIcon(task.priority);
    std::string typeDesc = getTaskTypeDescription(task.type);
    std::string statusDesc = getTaskStatusDescription(task.status);

    return priorityIcon + " [" + priorityDesc + "] " +
           task.id + ": " + typeDesc + " " +
           task.source + "→" + task.destination +
           " (" + statusDesc + ")";
}
```

## 验证测试

### 功能测试

1. ✅ 编译无错误
2. ✅ 时间显示格式正确
3. ✅ 状态面板布局合理
4. ✅ 任务列表美观易读

### 视觉测试

1. ✅ 不同窗口大小适配良好
2. ✅ 颜色搭配协调
3. ✅ 文字清晰可读
4. ✅ 交互反馈及时

### 性能测试

1. ✅ 界面响应流畅（>30 FPS）
2. ✅ 内存使用稳定
3. ✅ CPU 占用率合理

## 完成标准

- [ ] Toolbar 显示仅包含格式化的仿真时间
- [ ] StatusPanel 三个区域比例为 25:35:40
- [ ] 分隔线具有明显的视觉效果
- [ ] TaskListView 任务显示包含优先级颜色和详细信息
- [ ] 鼠标悬停任务项时有高亮效果
- [ ] 所有修改通过编译测试
- [ ] 程序运行稳定，无界面错误

## 预计时间

- **Toolbar 优化：** 2 小时
- **StatusPanel 调整：** 1 小时
- **TaskListView 美化：** 3 小时
- **测试验证：** 1 小时
- **总计：** 7 小时

---

_准备开始第一阶段实施！_
