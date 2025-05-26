# GUI 模拟仿真与数据流分析 (基于 GUITestMain.cpp)

本文档旨在详细分析 `GUITestMain.cpp` 如何驱动一个模拟的后端 (`GuiTestMockSimulation`) 与前端 GUI 组件进行交互，以便于理解 GUI 的开发流程和数据传递机制。分析将包含涉及的主要类、结构体、函数、变量及其作用和数据流向，并指出潜在的待完善或冗余部分。

## 1. 整体启动流程 (`src/test/GUITestMain.cpp`)

`GUITestMain.cpp` 作为测试 GUI 的入口点，其核心流程如下：

1.  **创建模拟后端**:
    - 实例化 `test::GuiTestMockSimulation`，这是一个实现了 `SimulationInterface` 接口的模拟后端。
    - `GuiTestMockSimulation` 内部会进一步实例化一个更底层的 `sim::MockSimulationInterface` (在 `GuiTestMockSimulation` 的构造函数中，通过 `m_mockSim` 成员)。
2.  **初始化模拟场景**:
    - 调用 `mockSimInterface->initializeDefaultScenario()` 来设置模拟环境的初始状态（例如，创建车辆、设备、任务等）。
3.  **创建主窗口**:
    - 实例化 `MainWindow`。
4.  **初始化主窗口**:
    - 调用 `mainWindow.initialize(mockSimInterface)`，将模拟后端的接口传递给主窗口。
    - 在 `MainWindow::initialize` 内部：
      - 保存 `SimulationInterface` 的共享指针。
      - 创建 SFML 窗口。
      - 加载全局字体。
      - 实例化各个 GUI 子组件：`SimulationView`、`StatusPanel`、`Toolbar`。
      - **关键**: 注册回调函数到 `SimulationInterface`，以便在后端数据更新时通知 `MainWindow`：
        - `registerStateUpdateCallback` -> `MainWindow::onSimulationStateUpdate`
        - `registerVehicleUpdateCallback` -> `MainWindow::onVehicleUpdate`
        - `registerDeviceUpdateCallback` -> `MainWindow::onDeviceUpdate`
      - 设置 `Toolbar` 的回调函数，将用户操作（如播放/暂停、重置、调整速度）传递回 `SimulationInterface`。
5.  **主事件循环**:
    - 持续运行，直到窗口关闭。
    - **处理 SFML 事件**:
      - 窗口关闭事件。
      - 窗口大小调整事件。
      - 将事件传递给 `mainWindow.processEvent(event)`，`MainWindow` 会进一步将事件分发给其子组件（如 `SimulationView` 处理鼠标拖拽、缩放等）。
    - **更新模拟状态**:
      - 调用 `mockSimInterface->update(dt)`，其中 `dt` 是自上一帧以来的时间差。这将驱动 `GuiTestMockSimulation` 更新其内部的 `sim::MockSimulationInterface` 的状态，并触发之前注册的回调函数，从而更新 GUI。
    - **渲染帧**:
      - 调用 `mainWindow.renderFrame()`，该函数会依次调用各子组件的 `draw` 方法。
      - 调用 `mainWindow.display()` 来显示渲染结果。

## 2. 模拟后端 (`test::GuiTestMockSimulation`)

- **文件**: `include/test/GuiTestMockSimulation.hpp`, `src/test/GuiTestMockSimulation.cpp`
- **核心职责**: 实现 `SimulationInterface`，作为 GUI 和更深层模拟逻辑 (`sim::MockSimulationInterface`) 之间的桥梁。它负责转换数据格式，管理模拟时间，并将用户操作转发给内部模拟。

### 2.1. 主要类和成员

- **`test::GuiTestMockSimulation` class**:
  - **`m_mockSim (sim::MockSimulationInterface)`**: 内部持有的更底层的模拟器实例。这个实例负责实际的模拟逻辑（车辆移动、任务分配等，尽管在当前 `sim::MockSimulationInterface` 中这些逻辑可能非常简化或随机）。
    - _潜在问题_: `sim::MockSimulationInterface` (位于 `include/gui/MockSimulationInterface.hpp`) 是一个更早期的、直接给 GUI 使用的模拟器。`GuiTestMockSimulation` 对其进行了封装。需要厘清这两层模拟的职责划分，是否存在功能重叠或不一致。从命名看，`GuiTestMockSimulation` 应该是专门为 `GUITestMain` 服务的，而 `sim::MockSimulationInterface` 可能是一个更通用的、可以被替换为真实后端的模拟接口。
  - **`m_currentState (SimulationInterface::SimulationState)`**: 缓存当前的整体仿真状态。
  - **`m_timeAccumulator (float)`**: 累积的仿真时间。
  - **Callbacks (`m_stateCallback`, `m_vehicleCallback`, `m_deviceCallback`)**: 存储由 `MainWindow` 注册的回调函数。

### 2.2. 关键函数

- **`GuiTestMockSimulation(float trackTotalLengthMm)` (Constructor)**:
  - 初始化 `m_mockSim` 和 `m_currentState` 的默认值。
- **`initializeDefaultScenario()`**:
  - 调用 `m_mockSim.initializeDefaultScenario()` 来设置 `sim::MockSimulationInterface` 的初始状态。
- **`update(sf::Time deltaTime)`**:
  - **作用**: 驱动模拟进行。
  - **逻辑**:
    1.  如果未暂停，则根据 `deltaTime` 和 `simulationSpeedFactor` 计算实际流逝的仿真时间 `dtSeconds`。
    2.  调用 `m_mockSim.update(sf::seconds(dtSeconds))` 来更新底层模拟。
    3.  更新 `m_currentState` (仿真时间、对象数量、任务统计)。
    4.  **触发回调**: 调用 `m_stateCallback`, `m_vehicleCallback`, `m_deviceCallback`，将最新的状态传递给 `MainWindow`。
- **`getSimulationState() const`**: 返回 `m_currentState`。
- **`getVehicleStates() const`**:
  - **作用**: 从 `m_mockSim` 获取原始车辆数据，转换为 `gui::VehicleState` 列表并返回。
  - **数据流**: `m_mockSim.getVehicles()` -> `convertToGuiVehicleState()` -> `std::vector<gui::VehicleState>`。
- **`getDeviceStates() const`**:
  - **作用**: 从 `m_mockSim` 获取原始设备数据，转换为 `gui::DeviceState` 列表并返回。
  - **数据流**: `m_mockSim.getDeviceStates()` -> `convertToGuiDeviceState()` -> `std::vector<gui::DeviceState>`。
- **`setSimulationSpeedFactor(float speedFactor)`**: 设置仿真速度因子，并触发状态更新回调。
- **`pauseSimulation()`, `resumeSimulation()`**: 控制仿真暂停/继续，并触发状态更新回调。
- **`resetSimulation()`**:
  - 调用 `m_mockSim.initializeDefaultScenario()` 重置底层模拟。
  - 重置 `m_currentState` 和 `m_timeAccumulator`。
  - 触发所有回调。
- **`register...Callback` functions**: 存储 `MainWindow` 提供的回调。
- **`getVehicleState(int vehicleId) const`**: 获取单个车辆状态，如果找不到则返回默认状态。
- **`getDeviceState(int deviceId) const`**: 获取单个设备状态，如果找不到则返回默认状态。
  - _潜在问题_: `getDeviceState` 中通过字符串解析从 `pair.first` (设备 ID 字符串) 中提取数字 ID 的逻辑比较复杂，且依赖于设备 ID 的命名约定 (如 "D1", "InputStation2")。如果 ID 格式不统一，可能会出错。直接使用 `std::map<int, sim::MockDeviceState>` 或在 `sim::MockDeviceState` 中存储数字 ID 可能更稳健。
- **`convertToGuiVehicleState(const sim::MockVehicle &mockVehicle) const`**:
  - **作用**: 将 `sim::MockVehicle` (来自 `m_mockSim`) 转换为 `gui::VehicleState`。
  - **关键转换**:
    - 速度单位: `mockVehicle.speedMmPerSecond` (mm/s) -> `gui::VehicleState::speed` (m/s)。
    - 状态: `mockVehicle.status (sim::VehicleStatus)` -> `convertSimToGuiVehicleStatus()` -> `gui::VehicleState::status (gui::VehicleStatus)`。
    - 任务 ID: `mockVehicle.currentTaskId` (int) -> `std::to_string` -> `gui::VehicleState::currentTaskId` (string)。
    - `isLoaded`: 根据 `mockVehicle.status` 推断。
    - `position`: 在 `gui::VehicleState` 中暂时使用 `placeholderPos(0,0)`，实际渲染位置由 `VehicleRenderer` 根据 `rawTrackPositionMm` 计算。
  - _待完善_: `isLoaded` 和 `cargoInfo` 的推断逻辑比较简单，可能与实际需求不符。`batteryLevel` 硬编码为 1.0f。
- **`convertToGuiDeviceState(const sim::MockDeviceState &mockDevice) const`**:
  - **作用**: 将 `sim::MockDeviceState` (来自 `m_mockSim`) 转换为 `gui::DeviceState`。
  - **关键转换**:
    - `deviceType`: 根据 `mockDevice.id` 字符串推断 (`INPUT_STATION`, `OUTPUT_STATION` 等)。
    - `status`: `mockDevice.operationalStatus (sim::DeviceOperationalStatus)` 和 `mockDevice.currentTaskCount` -> `convertSimToGuiDeviceStatus()` -> `gui::DeviceState::status (gui::DeviceStatus)`。
    - `position`: 同样使用 `placeholderPos(0,0)`，实际位置由 `WarehouseRenderer` 设置。
  - _待完善_: `deviceType` 的推断依赖于 ID 字符串，不够健壮。`gui::DeviceState` 中的 `capacity`, `currentLoad`, `materialId`, `processingProgress` 等字段未从 `mockDevice` 中获取，因为 `sim::MockDeviceState` 中没有这些信息。
- **Helper Converters**:
  - **`convertSimToGuiVehicleStatus(sim::VehicleStatus simStatus)`**: `sim::VehicleStatus` (enum) -> `gui::VehicleStatus` (enum)。
  - **`convertSimToGuiDeviceStatus(sim::DeviceOperationalStatus simOpStatus, int taskCount)`**: `sim::DeviceOperationalStatus` (enum) + `taskCount` -> `gui::DeviceStatus` (enum)。
    - _注意_: `gui::DeviceStatus::UNKNOWN_DEVICE_STATUS` 在 `SimObject.hpp` 中定义，而此函数中使用了 `gui::DeviceStatus::UNKNOWN_DEVICE_STATUS`。
- **`getTaskDetails(const std::string &taskId) const`**, **`getAllTasks() const`**:
  - 将 `m_mockSim` 中的 `sim::MockTask` 转换为 `Core::Task`。
  - 包含从字符串 ID 到数字 ID 的转换，以及基于描述推断任务类型的逻辑。
  - _待完善_: 任务类型和设备 ID 的解析逻辑同样依赖于字符串格式和约定，不够健壮。

### 2.3. `sim::MockSimulationInterface` (内部模拟层)

- **文件**: `include/gui/MockSimulationInterface.hpp` (注意路径，它不在 `test` 目录下，这可能暗示了其通用性)
- **核心职责**: 提供一个更底层的、可独立运行的模拟环境。`GuiTestMockSimulation` 是对它的封装和适配。
- **关键特性**:
  - 在单独的线程 (`m_updateThread`) 中运行 `updateLoop()`，模拟车辆移动和设备状态变化。
  - 使用 `std::mutex m_dataMutex` 来保护共享数据 (`m_vehicles`, `m_devices`) 的访问，这是为了解决之前提到的车辆闪烁问题。
  - `updateVehicles()`: 随机更新车辆位置和状态。
  - `updateDevices()`: 随机更新设备状态。
  - _潜在问题/设计选择_:
    - **随机性**: 大量的随机行为使得调试和可复现性变差。对于测试 GUI，有时确定性的、可编程的场景更有用。
    - **数据结构**: 直接使用 `gui::VehicleState` 和 `gui::DeviceState` 作为其内部存储。这意味着它与 GUI 层的数据结构紧密耦合。如果后端 API 发生变化，这里也需要改动。更理想的情况是，模拟后端使用自己独立的核心数据结构，然后通过适配层（如 `GuiTestMockSimulation` 中的 `convertToGui...` 函数）转换为 GUI 所需格式。
    - **回调机制**: 它自身也有一套回调机制 (`m_stateCallback`, `m_vehicleCallback`, `m_deviceCallback`)。`GuiTestMockSimulation` 实际上是透传了这些回调。

## 3. GUI 核心 (`MainWindow` 及其子组件)

### 3.1. `MainWindow`

- **文件**: `include/gui/MainWindow.hpp`, `src/gui/MainWindow.cpp`
- **核心职责**: 作为 GUI 的顶层容器，管理 SFML 窗口、事件处理、布局以及各个子视图（`SimulationView`, `StatusPanel`, `Toolbar`）。它连接了模拟后端 (`SimulationInterface`) 和 GUI 组件。

#### 3.1.1. 主要成员

- **`m_simInterface (std::shared_ptr<SimulationInterface>)`**: 指向模拟后端的接口。
- **`m_renderWindow (sf::RenderWindow)`**: SFML 窗口 (MainWindow 本身继承自 sf::RenderWindow)。
- **`m_globalFont (sf::Font)`**: 全局字体。
- **`m_simView (std::unique_ptr<SimulationView>)`**: 仿真视图。
- **`m_statusPanel (std::unique_ptr<StatusPanel>)`**: 状态面板。
- **`m_toolbar (std::unique_ptr<Toolbar>)`**: 工具栏。
- **`m_initialSize (sf::Vector2u)`**: 窗口初始大小。

#### 3.1.2. 关键函数

- **`initialize(std::shared_ptr<SimulationInterface> simInterface)`**:
  - **作用**: 设置 `m_simInterface`，创建窗口，加载资源，实例化子组件，并建立回调连接。
  - **数据流**: `simInterface` (来自 `GUITestMain`) 被保存。子组件被创建并传入必要的依赖（如字体、`simInterface` 的引用或指针）。
  - **回调注册**:
    - `m_simInterface->registerStateUpdateCallback(...)` -> `onSimulationStateUpdate`
    - `m_simInterface->registerVehicleUpdateCallback(...)` -> `onVehicleUpdate`
    - `m_simInterface->registerDeviceUpdateCallback(...)` -> `onDeviceUpdate`
  - `m_toolbar` 的回调设置，将用户输入转发给 `m_simInterface`。
- **`runEventLoop()` (在 `GUITestMain.cpp` 中实现)**: 处理事件、更新、渲染。
- **`processEvent(const sf::Event &event)` (在 `GUITestMain.cpp` 中调用 `handleSystemEvent`)**:
  - **`handleSystemEvent(const sf::Event &event)`**:
    - 处理窗口关闭、大小调整。
    - 将事件传递给 `m_simView->handleEvent()`、`m_statusPanel->handleEvent()`、`m_toolbar->handleEvent()`。
- **`renderFrame()`**:
  - 清空窗口。
  - 调用 `m_simView->draw()`, `m_statusPanel->draw()`, `m_toolbar->draw()`。
- **`updateLayout()`**: 当窗口大小改变时，重新计算和设置子组件的位置和大小。
- **`onSimulationStateUpdate(const SimulationInterface::SimulationState &state)`**:
  - **作用**: `SimulationInterface` 通知状态已更新。
  - **数据流**: `state` (来自 `SimulationInterface`) -> `m_statusPanel` (更新时间、计数等), `m_toolbar` (更新时间显示、播放/暂停按钮状态)。
  - 调用 `m_statusPanel->refreshContent()`，传递 `m_simView->getSelectedObject()` 和 `m_simInterface->getAllTasks()`。
- **`onVehicleUpdate(const std::vector<gui::VehicleState> &vehicles)`**:
  - **作用**: `SimulationInterface` 通知车辆状态已更新。
  - **数据流**: `vehicles` (来自 `SimulationInterface`) -> `m_simView->updateVehicles()`。
- **`onDeviceUpdate(const std::vector<gui::DeviceState> &devices)`**:
  - **作用**: `SimulationInterface` 通知设备状态已更新。
  - **数据流**: `devices` (来自 `SimulationInterface`) -> `m_simView->updateDevices()`。

### 3.2. `SimulationView`

- **文件**: `include/gui/SimulationView.hpp`, `src/gui/SimulationView.cpp`
- **核心职责**: 渲染 2D 仿真世界，包括轨道、车辆、仓库设备。处理用户与视图的交互（拖拽、缩放）。

#### 3.2.1. 主要成员

- **`m_worldView (sf::View)`**: 用于观察世界坐标系的视图。
- **`m_uiView (sf::View)`**: 用于 UI 叠加层（如果需要）。
- **`m_trackRenderer (TrackRenderer)`**: 轨道渲染器。
- **`m_vehicleRenderer (VehicleRenderer)`**: 车辆渲染器。
- **`m_warehouseRenderer (WarehouseRenderer)`**: 仓库/设备渲染器。
- **`m_allVehicles (std::vector<gui::VehicleState>)`**: 缓存的车辆状态。
- **`m_allDevices (std::vector<gui::DeviceState>)`**: 缓存的设备状态。
- **`m_selectedObject (gui::SimObject*)`**: 指向当前选中的对象。
- **`m_worldOriginOffsetPx (sf::Vector2f)`**: 世界原点在窗口中的像素偏移。
- **`m_zoomLevel (float)`**: 当前抽象缩放级别。
- **`m_unzoomedWorldViewSize (sf::Vector2f)`**: 未缩放时视图的大小。

#### 3.2.2. 关键函数

- **`initialize(sf::Vector2u windowSize, ...)`**:
  - 设置视图大小、中心。初始化 `m_trackRenderer`, `m_vehicleRenderer`, `m_warehouseRenderer`。
  - 计算初始缩放以适应轨道。
- **`updateVehicles(const std::vector<gui::VehicleState> &vehicles)`**:
  - **数据流**: `vehicles` (来自 `MainWindow`) -> `m_allVehicles` (缓存)。
  - 调用 `m_vehicleRenderer.updateStates(vehicles)`。
- **`updateDevices(const std::vector<gui::DeviceState> &devices)`**:
  - **数据流**: `devices` (来自 `MainWindow`) -> `m_allDevices` (缓存)。
  - 调用 `m_warehouseRenderer.updateStates(devices)`。
- **`handleEvent(const sf::Event &event, const sf::RenderWindow &window)`**:
  - 处理鼠标滚轮（缩放）、鼠标按下/松开/移动（拖拽、对象选择）。
  - 更新 `m_worldView` 的中心和大小。
  - **对象选择**:
    - 通过 `m_vehicleRenderer->getObjectAt(worldMousePos)` 和 `m_warehouseRenderer->getObjectAt(worldMousePos)` 检测鼠标下的对象。
    - 更新 `m_selectedObject`。
    - _注意_: 对象选择的优先级（车辆优先还是设备优先）取决于检测顺序。
- **`draw(sf::RenderTarget &target, sf::RenderStates states) const`**:
  - 设置 `m_worldView`。
  - 调用 `m_trackRenderer.draw()`, `m_warehouseRenderer.draw()`, `m_vehicleRenderer.draw()`。
  - 如果选中了对象，可能会绘制选中高亮（当前 `VehicleRenderer` 和 `WarehouseRenderer` 负责绘制各自的选中效果）。
- **`getSelectedObject() const`**: 返回 `m_selectedObject`。
- **`resetDefaultView()`**: 重置视图到初始状态。

### 3.3. `TrackRenderer`, `VehicleRenderer`, `WarehouseRenderer`

- **文件**: 各自的 `.hpp` 和 `.cpp` 文件。
- **核心职责**:
  - `TrackRenderer`: 根据轨道参数（长度、半径）生成和绘制轨道几何形状。
  - `VehicleRenderer`: 接收 `gui::VehicleState` 列表，根据车辆的 `rawTrackPositionMm` 计算其在轨道上的 2D 渲染位置和方向，并绘制车辆（通常是一个矩形或精灵）。处理车辆选中高亮。
  - `WarehouseRenderer`: 接收 `gui::DeviceState` 列表，根据设备在轨道上的逻辑位置（或预设的固定位置）计算其 2D 渲染位置，并绘制设备。处理设备选中高亮。
- **数据流**:
  - `SimulationView` 调用它们的 `updateStates()` (或类似方法) 传入最新的 `gui::VehicleState` 和 `gui::DeviceState`。
  - `SimulationView` 调用它们的 `draw()` 方法进行渲染。
  - `SimulationView` 调用它们的 `getObjectAt()` 方法进行对象拾取。
- **关键数据**:
  - `TrackRenderer` 使用 `m_trackLengthMm`, `m_curveRadiusMm`, `m_mmToPxRatio`, `m_scaleFactor`。
  - `VehicleRenderer` 依赖 `TrackRenderer` 的 `getPointAndOrientationOnCenterLine()` 来定位车辆。
  - `WarehouseRenderer` 也可能依赖 `TrackRenderer` 来定位与轨道相关的设备。
- _待完善/注意_:
  - 这些渲染器的初始化（特别是 `m_mmToPxRatio` 和 `m_scaleFactor` 的来源和一致性）对于正确缩放至关重要。
  - 车辆和设备的位置计算逻辑（从毫米单位到像素单位，从轨道位置到世界坐标）是核心且容易出错的部分。

### 3.4. `StatusPanel`

- **文件**: `include/gui/StatusPanel.hpp`, `src/gui/StatusPanel.cpp`
- **核心职责**: 显示整体仿真统计信息（时间、对象数量、任务计数）和选中对象的详细信息（通过 `ObjectInspector`）。

#### 3.4.1. 主要成员

- **`m_font (sf::Font&)`**: 字体。
- **Texts for stats**: `m_simTimeText`, `m_vehicleCountText`, `m_taskCountText` 等。
- **`m_objectInspector (ObjectInspector)`**: 用于显示选中对象详情的子组件。

#### 3.4.2. 关键函数

- **`StatusPanel(sf::Font &font, float width, float height)` (Constructor)**: 初始化文本和 `m_objectInspector`。
- **`refreshContent(const gui::SimObject *selectedObject, const std::vector<Core::Task> &allTasks)`**:
  - **作用**: 更新状态面板显示的内容。
  - **数据流**: `selectedObject` (来自 `SimulationView` via `MainWindow`) -> `m_objectInspector.updateObject()`。
  - `allTasks` (来自 `SimulationInterface` via `MainWindow`) 用于更新任务列表（如果 `StatusPanel` 直接显示任务的话，或者传递给更具体的任务列表视图）。当前 `StatusPanel` 似乎主要更新 `m_pendingTaskCountText`。
- **`setSimulationTime(float time)`, `setVehicleCount(int count)`, etc.**: 更新统计文本。
- **`draw(sf::RenderTarget &target, sf::RenderStates states) const`**: 绘制所有文本和 `m_objectInspector`。

### 3.5. `ObjectInspector`

- **文件**: `include/gui/ObjectInspector.hpp`, `src/gui/ObjectInspector.cpp`
- **核心职责**: 显示 `StatusPanel` 中当前选中的 `SimObject` 的详细属性。

#### 3.5.1. 主要成员

- **`m_currentObject (const gui::SimObject*)`**: 指向当前要显示的对象。
- **`m_titleText (sf::Text)`**: 标题。
- **`m_detailLines (std::vector<sf::Text>)`**: 显示对象属性的文本行。

#### 3.5.2. 关键函数

- **`updateObject(const gui::SimObject *selectedObject)`**:
  - **数据流**: `selectedObject` (来自 `StatusPanel`) -> `m_currentObject`。
  - 调用 `rebuildDisplay()`。
- **`rebuildDisplay()`**:
  - **核心逻辑**:
    1.  清空 `m_detailLines`。
    2.  如果 `m_currentObject` 为空，显示 "未选中对象"。
    3.  否则，根据 `m_currentObject->getType()` (即 `gui::SimObjectType::Vehicle` 或 `gui::SimObjectType::Device`)：
        - `static_cast` `m_currentObject` 到 `gui::VehicleState*` 或 `gui::DeviceState*`。
        - 使用 `addDetailLine()` 添加该类型对象的各个属性（ID, 类型, 状态, 位置, 速度, 任务 ID, 电量等）。
        - 使用 `vehicleStatusToString()`, `deviceTypeToString()`, `deviceStatusToString()` 将枚举转换为可读字符串。
- **`addDetailLine(const std::string &label, const std::string &value, float &currentY)`**: 辅助函数，创建并定位一行文本。
- **`draw(...)`**: 绘制标题和所有详情行。

### 3.6. `Toolbar`

- **文件**: `include/gui/Toolbar.hpp`, `src/gui/Toolbar.cpp`
- **核心职责**: 提供用户控制仿真行为的按钮（播放/暂停、重置视图、时间缩放等）和显示当前仿真时间。

#### 3.6.1. 主要成员

- **Buttons**: `m_playPauseButton`, `m_resetViewButton`, `m_timeScaleSlider` (或类似控件)。
- **`m_timeDisplay (sf::Text)`**: 显示仿真时间。
- **Callbacks**: `m_playPauseCallback`, `m_resetViewCallback`, `m_timeScaleCallback` 等，由 `MainWindow` 设置。

#### 3.6.2. 关键函数

- **`Toolbar(sf::Font &font, float width, float height)` (Constructor)**: 创建按钮和文本。
- **`handleEvent(const sf::Event &event, const sf::Vector2f &mousePos)`**:
  - 检测按钮点击或滑块操作。
  - 如果发生交互，调用相应的回调函数 (例如 `m_playPauseCallback()`)。
- **`updateTimeDisplay(float simTime, float realTime)`**: 更新时间文本。
- **`updatePlayPauseState(bool isPlaying)`**: 更新播放/暂停按钮的视觉状态。
- **`updateTimeScale(float scale)`**: 更新时间缩放滑块的视觉状态。
- **`set...Callback` functions**: 由 `MainWindow` 调用以设置回调。
- **`draw(...)`**: 绘制所有工具栏元素。

## 4. 核心数据结构 (`include/gui/SimObject.hpp`)

这个文件现在是 GUI 层面状态定义的核心。

- **`gui::SimObjectType` (enum)**: `Vehicle`, `Device`, `Track`.
- **`gui::SimObject` (class)**:
  - **成员**: `m_simObjType`, `m_id (std::string)`, `m_position (sf::Vector2f)`。
  - **作用**: 作为所有可在仿真视图中被选中和检视的对象的基类。提供通用属性。
- **`gui::VehicleStatus` (enum)**: `IDLE`, `MOVING_TO_LOAD`, `LOADING`, etc.
- **`gui::CargoDisplayInfo` (struct)**: `materialId`, `quantity`.
- **`gui::VehicleState : public SimObject` (struct)**:
  - **成员**: `rawTrackPositionMm`, `speed`, `status (gui::VehicleStatus)`, `currentTaskId`, `isLoaded`, `cargo`, `batteryLevel`.
  - `position` 成员已从 `SimObject` 继承。
- **`gui::DeviceType` (enum)**: `INPUT_STATION`, `OUTPUT_STATION`, `CORE_STORAGE_IN`, etc.
- **`gui::DeviceStatus` (enum)**: `IDLE`, `BUSY`, `WORKING`, `ERROR`, etc.
- **`gui::DeviceState : public SimObject` (struct)**:
  - **成员**: `deviceType`, `status (gui::DeviceStatus)`, `capacity`, `currentLoad`, `materialId`, `processingProgress`, `queuedTaskCount`, `boundVehicleId`.
  - `position` 成员已从 `SimObject` 继承。
- **`gui::WarehouseState` (struct)**:
  - **成员**: `id (int)`, `type (gui::DeviceType)`, `status (gui::DeviceStatus)`, `processingProgress`, `queuedTaskCount`, `position`.
  - _冗余/待评估_: 此结构体未继承 `SimObject`。其字段与 `DeviceState` 有部分重叠。在当前数据流中，`DeviceState` 是主要的设备表示。`WarehouseState` 的用途需要明确，如果无用则可以考虑移除以简化代码。**目前它已被合并到 `SimObject.hpp`，但其独立性（不继承`SimObject`）和与`DeviceState`的相似性仍值得注意。**
- **Helper functions**: `coreMotionToGuiStatus`, `coreToGuiDeviceType`, `coreToGuiDeviceStatus`.

## 5. 数据传递总结

1.  **模拟后端 -> GUI**:

    - `GuiTestMockSimulation::update()` 定期调用其内部 `sim::MockSimulationInterface::update()`。
    - `sim::MockSimulationInterface::updateLoop()` (在单独线程中) 或 `GuiTestMockSimulation::update()` (如果前者回调被后者包装) 修改内部的车辆和设备状态 (`m_vehicles`, `m_devices`，这些是 `std::vector<gui::VehicleState>` 和 `std::vector<gui::DeviceState>`)。
    - 然后通过 `m_vehicleCallback(m_vehicles)` 和 `m_deviceCallback(m_devices)` 将更新后的数据列表传递给 `MainWindow`。
    - `MainWindow` 的 `onVehicleUpdate` 和 `onDeviceUpdate` 将这些数据传递给 `SimulationView`。
    - `SimulationView` 更新其内部缓存，并通知其渲染器 (`VehicleRenderer`, `WarehouseRenderer`) 更新状态。
    - 整体仿真状态 (`SimulationState`) 也通过类似的回调 (`m_stateCallback`) 从 `GuiTestMockSimulation` 传递到 `MainWindow`，再分发到 `StatusPanel` 和 `Toolbar`。

2.  **GUI -> 模拟后端 (用户操作)**:

    - 用户与 `Toolbar` 交互（例如点击播放/暂停按钮）。
    - `Toolbar` 调用其对应的回调函数（例如 `m_playPauseCallback`）。
    - 这些回调函数在 `MainWindow::initialize` 中被设置为调用 `m_simInterface` 的相应方法（例如 `m_simInterface->pauseSimulation()` 或 `m_simInterface->resumeSimulation()`）。
    - `GuiTestMockSimulation` 接收到这些调用，并更新其 `m_currentState.isPaused` 或 `m_currentState.simulationSpeedFactor`，并可能将某些控制指令传递给其内部的 `m_mockSim`。

3.  **对象选择与显示**:
    - 用户在 `SimulationView` 中点击。
    - `SimulationView::handleEvent` 检测点击位置，并调用 `VehicleRenderer` 和 `WarehouseRenderer` 的 `getObjectAt()` 来确定选中的对象。
    - `SimulationView` 更新 `m_selectedObject`。
    - `MainWindow::onSimulationStateUpdate` (或其他合适的时机) 调用 `m_statusPanel->refreshContent(m_simView->getSelectedObject(), ...)`。
    - `StatusPanel` 将 `selectedObject` 传递给 `m_objectInspector->updateObject()`。
    - `ObjectInspector` 根据选中的对象类型和属性，动态构建并显示详细信息。

## 6. 潜在问题、冗余和待改进点

1.  **两层 Mock 模拟器**:

    - `GuiTestMockSimulation` 和其内部的 `sim::MockSimulationInterface` (来自 `include/gui/MockSimulationInterface.hpp`)。这种双层结构需要明确各自的职责。如果 `sim::MockSimulationInterface` 的目标是成为一个可替换的通用模拟接口，那么它不应该直接使用 `gui::` 前缀的类型，而应使用核心的、与 GUI 无关的类型。`GuiTestMockSimulation` 则负责将这些核心类型转换为 GUI 类型。
    - 当前 `sim::MockSimulationInterface` 直接使用了 `gui::VehicleState` 和 `gui::DeviceState`，这造成了与 GUI 的强耦合。

2.  **数据转换逻辑**:

    - 在 `GuiTestMockSimulation::convertToGuiVehicleState` 和 `convertToGuiDeviceState` 中，许多 GUI 状态字段是基于简单推断或硬编码的（如车辆的 `isLoaded`, `cargoInfo`, `batteryLevel`；设备的 `deviceType` 基于 ID 字符串，以及多个设备属性缺失）。这对于测试特定 GUI 功能可能不足。
    - ID 转换（字符串到数字，或从复杂字符串中提取数字）容易出错，应考虑更统一和健壮的 ID 管理。

3.  **`WarehouseState` 结构体**:

    - 如前所述，其在 `SimObject.hpp` 中的存在（不继承`SimObject`）和与 `DeviceState` 的功能重叠使其显得多余或用途不明。如果不再使用，应考虑移除。

4.  **`sim::MockSimulationInterface` 的随机性**:

    - 过度依赖随机数生成模拟行为，不利于 GUI 特定场景的稳定测试和调试。可以考虑增加可编程的场景设置接口。

5.  **头文件包含**:

    - 在合并 `DeviceState.hpp` 和 `WarehouseState.hpp` 到 `SimObject.hpp` 后，需要确保所有相关文件的 `#include` 都已正确更新。例如，`SimulationInterface.hpp` 中对 `VehicleState.hpp` 的包含现在可能是多余的，因为 `SimObject.hpp` (它现在应该被 `SimulationInterface.hpp` 包含以获取 `DeviceState`) 也定义了 `VehicleState`。这可能导致重定义问题，需要仔细检查。
    - `include/gui/MockSimulationInterface.hpp` 的路径暗示了它可能不仅仅用于测试，而是某种通用的模拟实现。这与其内部直接使用 `gui::` 类型相矛盾。

6.  **`DeviceState.cpp` 的构造函数**:

    - `DeviceState::DeviceState(const Core::DeviceBase &coreDevice, const sf::Vector2f &_pos)` 的实现位于 `src/gui/DeviceState.cpp`。在将 `DeviceState` 结构体移到 `SimObject.hpp` 后，这个 `.cpp` 文件如果仍然存在并且只包含这个构造函数的定义，需要确保其包含的头文件正确，并且其编译链接没有问题。如果 `DeviceState` 的所有成员和方法都可以在头文件中实现（例如都是内联或模板），则可以考虑移除这个 `.cpp` 文件。但由于它涉及到 `Core::DeviceBase` 到 `gui::DeviceState` 的转换，保留 `.cpp` 文件分离编译单元是合理的。

7.  **命名和路径约定**:

    - `sim::MockSimulationInterface` 位于 `include/gui/` 目录下，而 `test::GuiTestMockSimulation` 位于 `include/test/` 目录下。这有些混乱。通常，`sim::` 命名空间可能代表更核心或底层的模拟，而 `gui::` 代表 GUI 相关，`test::` 代表测试专用。

8.  **`ObjectInspector` 中的类型转换**:

    - 使用 `static_cast` 将 `SimObject*` 转换为 `VehicleState*` 或 `DeviceState*` 是基于 `getType()` 的结果。这是标准做法，但要确保 `getType()` 返回的类型与实际对象类型一致。

9.  **错误处理和日志**:
    - 在 `GuiTestMockSimulation` 的 `getTaskDetails` 和 `getAllTasks` 中，有 `std::cerr` 输出错误信息。对于一个库或测试工具，可能需要更灵活的日志记录机制。

## 7. 总结

`GUITestMain.cpp` 通过 `GuiTestMockSimulation` 驱动了一个与 GUI 分离的模拟后端。数据主要通过回调函数从后端推送到 `MainWindow`，再由 `MainWindow` 分发给各个子 GUI 组件进行显示。用户输入则通过 `Toolbar` 的回调传递给后端。核心数据结构如 `VehicleState` 和 `DeviceState` 在 `SimObject.hpp` 中定义，并在 GUI 的各个层面传递。

主要的改进方向在于明晰两层模拟器 (`GuiTestMockSimulation` 和 `sim::MockSimulationInterface`) 的职责，减少模拟后端与 GUI 特定类型的耦合，增强模拟场景的可控性，以及清理冗余代码和统一命名约定。
