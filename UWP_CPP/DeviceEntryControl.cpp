#include "pch.h"
#include "DeviceEntryControl.h"
#include "DeviceEntryControl.g.cpp"

namespace winrt::UWP_CPP::implementation
{
    template<typename TDevice>
    struct DeviceEntryT : DeviceEntry
    {
    protected:
        TDevice m_device;
        uint64_t m_timestamp;

    public:
        DeviceEntryT(TDevice device)
            : m_device(device)
        {
        }

        uint16_t VendorId() { return m_device.VendorId(); }
        uint16_t ProductId() { return m_device.ProductId(); }
        Custom::GameControllerVersionInfo HardwareVersion() { return m_device.HardwareVersion(); }
        Custom::GameControllerVersionInfo FirmwareVersion() { return m_device.FirmwareVersion(); }
    };

    struct HidDeviceEntry : DeviceEntryT<WGIC::HidDevice>
    {
        HidDeviceEntry(WGIC::HidDevice device)
            : DeviceEntryT(device)
        {
        }

        winrt::hstring DeviceType()
        {
            return L"HID Device";
        }

        winrt::hstring GetNextEvent()
        {
            uint64_t timestamp;
            uint8_t reportId;
            winrt::com_array<uint8_t> reportBuffer;
            m_device.GetLatestReport(timestamp, reportId, reportBuffer);

            if (timestamp != m_timestamp)
            {
                m_timestamp = timestamp;
                std::wstring event = fmt::format(L"Timestamp: {}, ID: 0x{:02X}, length: {}\n{}\n",
                    timestamp, reportId, reportBuffer.size(),
                    Utilities::BytesToHex(reportBuffer.data(), reportBuffer.size())
                );
                return winrt::hstring(event);
            }
            else
            {
                return winrt::hstring();
            }

        }
    };

    struct XusbDeviceEntry : DeviceEntryT<WGIC::XusbDevice>
    {
        XusbDeviceEntry(WGIC::XusbDevice device)
            : DeviceEntryT(device)
        {
        }

        winrt::hstring DeviceType()
        {
            return L"XUSB Device";
        }

        winrt::hstring GetNextEvent()
        {
            uint64_t timestamp;
            uint8_t reportId;
            winrt::com_array<uint8_t> reportBuffer;
            m_device.GetLatestInput(timestamp, reportId, reportBuffer);

            if (timestamp != m_timestamp)
            {
                m_timestamp = timestamp;

                std::wstring event = fmt::format(L"Timestamp: {}, ID: 0x{:02X}, length: {}\n{}\n",
                    timestamp, reportId, reportBuffer.size(),
                    Utilities::BytesToHex(reportBuffer.data(), reportBuffer.size())
                );

                return winrt::hstring(event);
            }
            else
            {
                return winrt::hstring();
            }
        }
    };

    struct GipDeviceEntry : DeviceEntryT<WGIC::GipDevice>
    {
        GipDeviceEntry(WGIC::GipDevice device)
            : DeviceEntryT(device)
        {
        }

        winrt::hstring DeviceType()
        {
            return L"GIP Device";
        }

        winrt::hstring GetNextEvent()
        {
            uint64_t timestamp;
            Custom::GipMessageClass messageClass;
            uint8_t messageId;
            uint8_t sequenceId;
            winrt::com_array<uint8_t> messageBuffer;
            m_device.GetLatestMessage(timestamp, messageClass, messageId, sequenceId, messageBuffer);

            if (timestamp != m_timestamp)
            {
                m_timestamp = timestamp;

                std::wstring messageClassName;
                switch (messageClass)
                {
                case Custom::GipMessageClass::Command: messageClassName = L"Command"; break;
                case Custom::GipMessageClass::LowLatency: messageClassName = L"LowLatency"; break;
                case Custom::GipMessageClass::StandardLatency: messageClassName = L"StandardLatency"; break;
                default: messageClassName = std::to_wstring((uint32_t)messageClass); break;
                }

                std::wstring event = fmt::format(L"Timestamp: {}, class: {}, ID: 0x{:02X}, sequence: {}, length: {}\n{}\n",
                    timestamp, messageClassName, messageId, sequenceId, messageBuffer.size(),
                    Utilities::BytesToHex(messageBuffer.data(), messageBuffer.size())
                );

                return winrt::hstring(event);
            }
            else
            {
                return winrt::hstring();
            }
        }
    };

    DeviceEntryControl::DeviceEntryControl(Input::IGameController device)
    {
        auto hid = device.try_as<WGIC::HidDevice>();
        if (hid != nullptr)
        {
            m_entry = std::make_unique<HidDeviceEntry>(hid);
            return;
        }

        auto xusb = device.try_as<WGIC::XusbDevice>();
        if (xusb != nullptr)
        {
            m_entry = std::make_unique<XusbDeviceEntry>(xusb);
            return;
        }

        auto gip = device.try_as<WGIC::GipDevice>();
        if (gip != nullptr)
        {
            m_entry = std::make_unique<GipDeviceEntry>(gip);
            return;
        }

        auto logger = spdlog::get(s_loggerName)->clone("DeviceEntryControl::ctor");
        logger->debug("Invalid device received!");
        Utilities::LogIInspectable(logger, device, spdlog::level::err);
        throw hresult_invalid_argument();
    }

    void DeviceEntryControl::InitializeComponent()
    {
        DeviceEntryControlT::InitializeComponent();

        titleText().Text(m_entry->DeviceType());
        auto hardware = m_entry->HardwareVersion();
        auto firmware = m_entry->FirmwareVersion();
        infoText().Text(fmt::format(L"Vendor ID: 0x{:04X}\nProduct ID: 0x{:04X}\n"
            "Hardware Version: v{}.{}.{}.{}\nFirmware Version: v{}.{}.{}.{}",
            m_entry->VendorId(), m_entry->ProductId(),
            hardware.Major, hardware.Minor, hardware.Build, hardware.Revision,
            firmware.Major, firmware.Minor, firmware.Build, firmware.Revision
        ));
    }

    winrt::event_token DeviceEntryControl::SelectButtonClicked(Xaml::RoutedEventHandler const& handler)
    {
        return m_selectButtonClicked.add(handler);
    }

    void DeviceEntryControl::SelectButtonClicked(winrt::event_token const& token) noexcept
    {
        m_selectButtonClicked.remove(token);
    }

    winrt::hstring DeviceEntryControl::DeviceType()
    {
        return m_entry->DeviceType();
    }

    uint16_t DeviceEntryControl::VendorId()
    {
        return m_entry->VendorId();
    }

    uint16_t DeviceEntryControl::ProductId()
    {
        return m_entry->ProductId();
    }

    Custom::GameControllerVersionInfo DeviceEntryControl::HardwareVersion()
    {
        return m_entry->HardwareVersion();
    }

    Custom::GameControllerVersionInfo DeviceEntryControl::FirmwareVersion()
    {
        return m_entry->FirmwareVersion();
    }

    winrt::hstring DeviceEntryControl::GetNextEvent()
    {
        return m_entry->GetNextEvent();
    }

    void DeviceEntryControl::selectButton_Clicked(Foundation::IInspectable const&, Xaml::RoutedEventArgs const& args)
    {
        m_selectButtonClicked(*this, args);
    }
}
