#include "../../include/Core/Device.hpp"
#include "../../include/gui/DeviceState.hpp"

namespace gui
{

    DeviceState::DeviceState(const Core::DeviceBase &coreDevice, const sf::Vector2f &_pos)
        : SimObject(SimObjectType::Device, std::to_string(coreDevice.getId()), _pos),
          deviceType(coreToGuiDeviceType(coreDevice.getCoreType())),
          status(coreToGuiDeviceStatus(coreDevice.getCoreStatus())),
          capacity(coreDevice.getCapacity()),
          currentLoad(coreDevice.getCurrentLoad()),
          materialId(coreDevice.getCurrentMaterialId()),
          processingProgress(coreDevice.getProcessingProgress()),
          queuedTaskCount(static_cast<int>(coreDevice.getQueuedTaskCount())),
          boundVehicleId(""),
          position(_pos)
    {
    }

} // namespace gui