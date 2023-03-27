#include "pch.h"
#include "MainPage.h"
#include "MainPage.g.cpp"

namespace winrt::UWP_CPP::implementation
{
    void MainPage::Page_Loaded(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        RefreshDevices();

        m_hidDeviceAddedToken = WGIC::HidDevice::DeviceAdded({ this, &MainPage::OnHidDeviceListChanged });
        m_hidDeviceRemovedToken = WGIC::HidDevice::DeviceRemoved({ this, &MainPage::OnHidDeviceListChanged });
        m_xusbDeviceAddedToken = WGIC::XusbDevice::DeviceAdded({ this, &MainPage::OnXusbDeviceListChanged });
        m_xusbDeviceRemovedToken = WGIC::XusbDevice::DeviceRemoved({ this, &MainPage::OnXusbDeviceListChanged });
        m_gipDeviceAddedToken = WGIC::GipDevice::DeviceAdded({ this, &MainPage::OnGipDeviceListChanged });
        m_gipDeviceRemovedToken = WGIC::GipDevice::DeviceRemoved({ this, &MainPage::OnGipDeviceListChanged });

        m_eventThread = std::thread(&MainPage::EntryEventThread, this);
    }
    
    void MainPage::Page_Unloaded(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        WGIC::HidDevice::DeviceAdded(m_hidDeviceAddedToken);
        WGIC::HidDevice::DeviceRemoved(m_hidDeviceRemovedToken);
        WGIC::XusbDevice::DeviceAdded(m_xusbDeviceAddedToken);
        WGIC::XusbDevice::DeviceRemoved(m_xusbDeviceRemovedToken);
        WGIC::GipDevice::DeviceAdded(m_gipDeviceAddedToken);
        WGIC::GipDevice::DeviceRemoved(m_gipDeviceRemovedToken);

        SetEvent(m_threadStop.get());
        m_eventThread.join();
    }

    void MainPage::AddDevice(Input::IGameController device)
    {
        if (!device)
            return;

        TestApp::DeviceEntryControl entry(device);
        entry.SelectButtonClicked({ this, &MainPage::OnEntrySelected });
        availableDevices().Children().Append(entry);
    }

    void MainPage::SelectEntry(TestApp::DeviceEntryControl entry)
    {
        std::lock_guard<std::mutex> lock(m_entryLock);
        m_currentEntry = entry;
        deviceEvents().Text(L"");

        if (entry)
        {
            auto hardware = entry.HardwareVersion();
            auto firmware = entry.FirmwareVersion();
            currentDeviceInfo().Text(fmt::format(L"Device type: {}\nVendor ID: 0x{:04X}\nProduct ID: 0x{:04X}\n"
                "Hardware Version: v{}.{}.{}.{}\nFirmware Version: v{}.{}.{}.{}",
                entry.DeviceType(), entry.VendorId(), entry.ProductId(),
                hardware.Major, hardware.Minor, hardware.Build, hardware.Revision,
                firmware.Major, firmware.Minor, firmware.Build, firmware.Revision
            ));
        }
        else
        {
            currentDeviceInfo().Text(L"Device type: \nVendor ID: \nProduct ID: \nHardware Version: \nFirmware Version: ");
        }
    }

    void MainPage::RefreshDevices()
    {
        SelectEntry(nullptr);

        std::lock_guard<std::mutex> lock(m_devicesLock);
        availableDevices().Children().Clear();

        auto hidDevices = WGIC::HidDevice::Devices();
        if (hidDevices != nullptr)
        {
            for (auto device : hidDevices)
            {
                AddDevice(device);
            }
        }

        auto xusbDevices = WGIC::XusbDevice::Devices();
        if (xusbDevices != nullptr)
        {
            for (auto device : xusbDevices)
            {
                AddDevice(device);
            }
        }

        auto gipDevices = WGIC::GipDevice::Devices();
        if (gipDevices != nullptr)
        {
            for (auto device : gipDevices)
            {
                AddDevice(device);
            }
        }
    }

    void MainPage::EntryEventThread()
    {
        while (WaitForSingleObjectEx(m_threadStop.get(), 0, false) == WAIT_TIMEOUT)
        {
            winrt::hstring event;
            {
                std::lock_guard<std::mutex> lock(m_entryLock);
                if (m_currentEntry == nullptr)
                    continue;

                event = m_currentEntry.GetNextEvent();
            }
            if (event.empty())
                continue;

            // Update UI
            Dispatcher().RunAsync(UI::Core::CoreDispatcherPriority::Normal, [&]{
                // Write event to textbox
                auto text = deviceEvents().Text();
                deviceEvents().Text(text + event);

                // Scroll to the bottom of the textbox
                deviceEventsScroll().ChangeView(nullptr, deviceEventsScroll().ScrollableHeight() + 1.0, nullptr, true);
            }).get();
        }
    }

    void MainPage::OnHidDeviceListChanged(Foundation::IInspectable const&, WGIC::HidDevice const&)
    {
        Dispatcher().RunAsync(UI::Core::CoreDispatcherPriority::Normal, { this, &MainPage::RefreshDevices });
    }

    void MainPage::OnXusbDeviceListChanged(Foundation::IInspectable const&, WGIC::XusbDevice const&)
    {
        Dispatcher().RunAsync(UI::Core::CoreDispatcherPriority::Normal, { this, &MainPage::RefreshDevices });
    }

    void MainPage::OnGipDeviceListChanged(Foundation::IInspectable const&, WGIC::GipDevice const&)
    {
        Dispatcher().RunAsync(UI::Core::CoreDispatcherPriority::Normal, { this, &MainPage::RefreshDevices });
    }

    void MainPage::OnEntrySelected(Foundation::IInspectable const& sender, Xaml::RoutedEventArgs const&)
    {
        auto entry = sender.try_as<TestApp::DeviceEntryControl>();
        if (entry == nullptr)
            return;

        SelectEntry(entry);
    }

    void MainPage::reenumerateButton_Click(Foundation::IInspectable const&, Xaml::RoutedEventArgs const&)
    {
        RefreshDevices();
    }
}
