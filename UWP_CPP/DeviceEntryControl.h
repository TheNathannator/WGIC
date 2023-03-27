#pragma once
#include "DeviceEntryControl.g.h"

namespace winrt::UWP_CPP::implementation
{
    struct DeviceEntry
    {
    public:
        virtual ~DeviceEntry() { };

        virtual winrt::hstring DeviceType() = 0;
        virtual uint16_t VendorId() = 0;
        virtual uint16_t ProductId() = 0;
        virtual Custom::GameControllerVersionInfo HardwareVersion() = 0;
        virtual Custom::GameControllerVersionInfo FirmwareVersion() = 0;
        virtual winrt::hstring GetNextEvent() = 0;
    };

    struct DeviceEntryControl : DeviceEntryControlT<DeviceEntryControl>
    {
    private:
        std::unique_ptr<DeviceEntry> m_entry;
        winrt::event<Xaml::RoutedEventHandler> m_selectButtonClicked {};

    public:
        DeviceEntryControl(Input::IGameController device);
        void InitializeComponent();

        winrt::event_token SelectButtonClicked(Xaml::RoutedEventHandler const& handler);
        void SelectButtonClicked(winrt::event_token const& token) noexcept;

        winrt::hstring DeviceType();
        uint16_t VendorId();
        uint16_t ProductId();
        Custom::GameControllerVersionInfo HardwareVersion();
        Custom::GameControllerVersionInfo FirmwareVersion();
        winrt::hstring GetNextEvent();

        void selectButton_Clicked(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::UWP_CPP::factory_implementation
{
    struct DeviceEntryControl : DeviceEntryControlT<DeviceEntryControl, implementation::DeviceEntryControl>
    {
    };
}
