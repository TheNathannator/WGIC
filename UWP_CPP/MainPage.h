#pragma once
#include "MainPage.g.h"

namespace winrt::UWP_CPP::implementation
{
    struct MainPage : MainPageT<MainPage>
    {
    public:
        MainPage() = default;

        void Page_Loaded(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const& args);
        void Page_Unloaded(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const& args);

        void reenumerateButton_Click(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const& args);

    private:
        winrt::event_token m_hidDeviceAddedToken;
        winrt::event_token m_hidDeviceRemovedToken;
        winrt::event_token m_xusbDeviceAddedToken;
        winrt::event_token m_xusbDeviceRemovedToken;
        winrt::event_token m_gipDeviceAddedToken;
        winrt::event_token m_gipDeviceRemovedToken;

        std::mutex m_devicesLock;
        std::mutex m_entryLock;

        TestApp::DeviceEntryControl m_currentEntry { nullptr };
        std::thread m_eventThread;
        winrt::handle m_threadStop { CreateEvent(nullptr, true, false, nullptr) };

        void AddDevice(Input::IGameController device);
        void SelectEntry(TestApp::DeviceEntryControl entry);
        void RefreshDevices();

        void EntryEventThread();

        void OnHidDeviceListChanged(Foundation::IInspectable const& sender, WGIC::HidDevice const& device);
        void OnXusbDeviceListChanged(Foundation::IInspectable const& sender, WGIC::XusbDevice const& device);
        void OnGipDeviceListChanged(Foundation::IInspectable const& sender, WGIC::GipDevice const& device);
        void OnEntrySelected(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const& args);
    };
}

namespace winrt::UWP_CPP::factory_implementation
{
    struct MainPage : MainPageT<MainPage, implementation::MainPage>
    {
    };
}
