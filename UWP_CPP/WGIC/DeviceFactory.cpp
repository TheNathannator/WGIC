#include "pch.h"
#include "WGIC/HidDevice.h"
#include "WGIC/XusbDevice.h"
#include "WGIC/GipDevice.h"
#include "WGIC/DeviceFactory.h"

namespace WGIC_impl = winrt::WGIC::implementation;

namespace winrt::WGIC
{
    Custom::ICustomGameControllerFactory DeviceFactory::s_factory = winrt::make<WGIC::DeviceFactory>();

    void DeviceFactory::RegisterHardwareIds(uint16_t vendorId, uint16_t productId)
    {
        Custom::GameControllerFactoryManager::RegisterCustomFactoryForHardwareId(s_factory, vendorId, productId);
    }

    void DeviceFactory::RegisterXusbType(Custom::XusbDeviceType type, Custom::XusbDeviceSubtype subtype)
    {
        Custom::GameControllerFactoryManager::RegisterCustomFactoryForXusbType(s_factory, type, subtype);
    }

    void DeviceFactory::RegisterGipInterfaceGuid(winrt::guid const& interfaceGuid)
    {
        Custom::GameControllerFactoryManager::RegisterCustomFactoryForGipInterface(s_factory, interfaceGuid);
    }

    Foundation::IInspectable DeviceFactory::CreateGameController(Custom::IGameControllerProvider const& provider)
    {
        auto logger = spdlog::get(s_loggerName)->clone("DeviceFactory::CreateGameController");
        Utilities::LogIInspectable(logger, provider);

        auto hidProvider = provider.try_as<Custom::HidGameControllerProvider>();
        if (hidProvider)
        {
            return winrt::make<WGIC_impl::HidDevice>(hidProvider);
        }

        auto xusbProvider = provider.try_as<Custom::XusbGameControllerProvider>();
        if (xusbProvider)
        {
            return winrt::make<WGIC_impl::XusbDevice>(xusbProvider);
        }

        auto gipProvider = provider.try_as<Custom::GipGameControllerProvider>();
        if (gipProvider)
        {
            return winrt::make<WGIC_impl::GipDevice>(gipProvider);
        }

        logger->error("Invalid provider received!");
        throw winrt::hresult_invalid_argument();
    }

    void DeviceFactory::OnGameControllerAdded(Input::IGameController const& controller)
    {
        auto logger = spdlog::get(s_loggerName)->clone("DeviceFactory::OnGameControllerAdded");
        Utilities::LogIInspectable(logger, controller);

        auto hidDevice = controller.try_as<WGIC::HidDevice>();
        if (hidDevice)
        {
            WGIC_impl::HidDevice::AddDevice(hidDevice);
            return;
        }

        auto xusbDevice = controller.try_as<WGIC::XusbDevice>();
        if (xusbDevice)
        {
            WGIC_impl::XusbDevice::AddDevice(xusbDevice);
            return;
        }

        auto gipDevice = controller.try_as<WGIC::GipDevice>();
        if (gipDevice)
        {
            WGIC_impl::GipDevice::AddDevice(gipDevice);
            return;
        }

        logger->error("Invalid device received!");
        throw winrt::hresult_invalid_argument();
    }

    void DeviceFactory::OnGameControllerRemoved(Input::IGameController const& controller)
    {
        auto logger = spdlog::get(s_loggerName)->clone("DeviceFactory::OnGameControllerRemoved");
        Utilities::LogIInspectable(logger, controller);

        auto hidDevice = controller.try_as<WGIC::HidDevice>();
        if (hidDevice)
        {
            WGIC_impl::HidDevice::RemoveDevice(hidDevice);
            return;
        }

        auto xusbDevice = controller.try_as<WGIC::XusbDevice>();
        if (xusbDevice)
        {
            WGIC_impl::XusbDevice::RemoveDevice(xusbDevice);
            return;
        }

        auto gipDevice = controller.try_as<WGIC::GipDevice>();
        if (gipDevice)
        {
            WGIC_impl::GipDevice::RemoveDevice(gipDevice);
            return;
        }

        logger->error("Invalid device received!");
        throw winrt::hresult_invalid_argument();
    }
}