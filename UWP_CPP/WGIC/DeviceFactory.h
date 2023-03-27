#pragma once
#include "pch.h"

namespace winrt::WGIC
{
    struct DeviceFactory : winrt::implements<DeviceFactory, Custom::ICustomGameControllerFactory>
    {
    private:
        static Custom::ICustomGameControllerFactory s_factory;

    public:
        static void RegisterHardwareIds(uint16_t vendorId, uint16_t productId);
        static void RegisterXusbType(Custom::XusbDeviceType type, Custom::XusbDeviceSubtype subtype);
        static void RegisterGipInterfaceGuid(winrt::guid const& interfaceGuid);

        template<typename TDevice>
        static TDevice FromGameController(Input::IGameController const& gameController)
        {
            return Custom::GameControllerFactoryManager::TryGetFactoryControllerFromGameController(s_factory, gameController)
                .try_as<TDevice>(); // try_as will return null if it is already null
        }

        Foundation::IInspectable CreateGameController(Custom::IGameControllerProvider const& provider);
        void OnGameControllerAdded(Input::IGameController const& controller);
        void OnGameControllerRemoved(Input::IGameController const& controller);
    };
}