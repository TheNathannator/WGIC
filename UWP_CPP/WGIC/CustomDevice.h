#pragma once
#include "pch.h"
#include "WGIC/DeviceFactory.h"
#include "WGIC/VectorCollection.h"

namespace winrt::WGIC
{
    template<typename D, typename TDevice, typename... I>
    using CustomDeviceT = winrt::implements<D, TDevice, Custom::IGameControllerInputSink,
        winrt::cloaked<Custom::IAggregable>, I...>;
    // Cloaking the IAggregable interface is optional, all it does is keep it from being listed in GetIids.

    template<typename D, typename TDevice, typename TProvider, typename... I>
    struct __declspec(empty_bases) CustomDevice : CustomDeviceT<D, TDevice, I...>
    {
        // The cppwinrt-generated base is specifically ignored in order to correctly support the aggregation that's
        // necessary for custom devices. IGameController is not implemented directly, instead an aggregation pattern is
        // used. C++/WinRT provides the winrt::composing marker type for this, however that is also ignored since there
        // are some unfortunate infinite loop issues that require this to be handled manually.

    protected:
        static inline Collections::IVector<TDevice> s_devices = winrt::make<WGIC::VectorCollection<TDevice>>();
        static inline std::mutex s_devicesLock {};
        static inline winrt::event<Foundation::EventHandler<TDevice>> s_deviceAdded {};
        static inline winrt::event<Foundation::EventHandler<TDevice>> s_deviceRemoved {};

    public:
        static Collections::IVectorView<TDevice> Devices()
        {
            std::lock_guard<std::mutex> lock(s_devicesLock);
            return s_devices.GetView();
        }

        static winrt::event_token DeviceAdded(Foundation::EventHandler<TDevice> const& handler)
        {
            return s_deviceAdded.add(handler);
        }

        static void DeviceAdded(winrt::event_token const& token) noexcept
        {
            s_deviceAdded.remove(token);
        }

        static winrt::event_token DeviceRemoved(Foundation::EventHandler<TDevice> const& handler)
        {
            return s_deviceRemoved.add(handler);
        }

        static void DeviceRemoved(winrt::event_token const& token) noexcept
        {
            s_deviceRemoved.remove(token);
        }

        static void AddDevice(TDevice const& device)
        {
            if (!device)
                return;

            // New scope is done to prevent deadlocking if the
            // event subscriber gets the device list in the event handler
            bool fireEvent = false;
            {
                std::lock_guard<std::mutex> lock(s_devicesLock);
                uint32_t index = 0;
                if (!s_devices.IndexOf(device, index))
                {
                    fireEvent = true;
                    s_devices.Append(device);
                }
            }

            if (fireEvent)
                s_deviceAdded(nullptr, device);
        }

        static void RemoveDevice(TDevice const& device)
        {
            if (!device)
                return;

            // New scope is done to prevent deadlocking if the
            // event subscriber gets the device list in the event handler
            bool fireEvent = false;
            {
                std::lock_guard<std::mutex> lock(s_devicesLock);
                uint32_t index = 0;
                if (s_devices.IndexOf(device, index))
                {
                    fireEvent = true;
                    s_devices.RemoveAt(index);
                }
            }

            if (fireEvent)
                s_deviceRemoved(nullptr, device);
        }

        static TDevice FromGameController(Input::IGameController const& gameController)
        {
            return DeviceFactory::FromGameController<TDevice>(gameController);
        }

    private:
        Foundation::IInspectable m_innerObject { nullptr };

    protected:
        TProvider m_provider;

        std::mutex m_readingLock;
        bool m_inputSuspended;

    public:
        CustomDevice(TProvider const& provider)
            : m_provider(provider)
        {
        }

        uint16_t VendorId()
        {
            return m_provider.HardwareVendorId();
        }

        uint16_t ProductId()
        {
            return m_provider.HardwareProductId();
        }

        Custom::GameControllerVersionInfo HardwareVersion()
        {
            return m_provider.HardwareVersionInfo();
        }

        Custom::GameControllerVersionInfo FirmwareVersion()
        {
            return m_provider.FirmwareVersionInfo();
        }

        bool IsConnected()
        {
            return m_provider.IsConnected();
        }

        void OnInputSuspended(uint64_t timestamp)
        {
#ifdef _DEBUG
            auto logger = spdlog::get(s_loggerName)->clone("CustomDevice::OnInputSuspended");
            logger->debug("Input suspended at {}", timestamp);
#endif

            std::lock_guard<std::mutex> lock(m_readingLock);
            m_inputSuspended = true;
        }

        void OnInputResumed(uint64_t timestamp)
        {
#ifdef _DEBUG
            auto logger = spdlog::get(s_loggerName)->clone("CustomDevice::OnInputResumed");
            logger->debug("Input resumed at {}", timestamp);
#endif

            std::lock_guard<std::mutex> lock(m_readingLock);
            m_inputSuspended = false;
        }

        // Modified from winrt::root_implements::QueryInterface to avoid an infinite loop
        int32_t QueryInterface(winrt::guid const& id, void** object) noexcept
        {
            // Check for IGameController explicitly to avoid an infinite loop
            // from neither us nor the inner object supporting an interface
            if (winrt::is_guid_of<Input::IGameController>(id) && m_innerObject)
                return m_innerObject.as(id, object);

            return CustomDeviceT<D, TDevice, I...>::QueryInterface(id, object);
        }

        // Modified from winrt::root_implements::NonDelegatingGetIids to include IGameController in the IIDs list
        // It's not implemented directly but it should still be included in the list
        impl::hresult_type GetIids(impl::count_type* count, impl::guid_type** array) noexcept
        {
            // This is probably already handled but it's better safe than sorry
            if (!count || !array)
            {
                return impl::error_invalid_argument;
            }

            using interfaces = impl::filter<impl::is_uncloaked_interface, D, Input::IGameController>;
            using iids = impl::uncloaked_iids<interfaces>;

            auto local_count = static_cast<impl::count_type>(iids::value.size());
            if (local_count > 0)
            {
                auto local_iids = reinterpret_cast<impl::guid_type const*>(iids::value.data());
                *count = local_count;
                *array = static_cast<impl::guid_type*>(CoTaskMemAlloc(sizeof(impl::guid_type) * local_count));
                if (*array == nullptr)
                {
                    return impl::error_bad_alloc;
                }
                std::copy(local_iids, local_iids + local_count, *array);
            }
            else
            {
                *count = 0;
                *array = nullptr;
            }

            return 0;
        }

        // Aggregates an inner object to this object
        void Aggregate(Foundation::IInspectable object)
        {
            auto logger = spdlog::get(s_loggerName)->clone("CustomDevice::Aggregate");
            if (!object)
            {
                logger->error("Invalid object received!");
                throw winrt::hresult_invalid_argument();
            }

            // Note: I wasn't able to find a good way to check if we're being passed ourselves,
            // since this is just a demonstration I can't be bothered to spend much on it lol

            if (m_innerObject)
            {
                logger->error("Already aggregated!");
                throw winrt::hresult_error(CLASS_E_NOAGGREGATION);
            }

            m_innerObject = object;
            logger->debug("Inner object added");

            // This causes an access violation crash here unfortunately
            // The object we're being handed does not have its own IInspectable data and instead uses us for that,
            // but it hasn't actually assigned us as its outer object yet
            // Utilities::LogIInspectable(logger, object);
        }
    };
}