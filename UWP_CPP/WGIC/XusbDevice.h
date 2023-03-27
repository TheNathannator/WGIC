#pragma once
#include "pch.h"
#include "WGIC.XusbDevice.g.h"
#include "WGIC/CustomDevice.h"

namespace winrt::WGIC::implementation
{
    struct XusbDevice : WGIC::CustomDevice<XusbDevice, WGIC::XusbDevice, Custom::XusbGameControllerProvider,
        Custom::IXusbGameControllerInputSink>
    {
    public:
        static void RegisterType(Custom::XusbDeviceType type, Custom::XusbDeviceSubtype subtype);

    private:
        uint64_t m_currentTimestamp = 0;
        uint8_t m_currentReportId = 0;
        winrt::com_array<uint8_t> m_currentReport;

    public:
        XusbDevice(Custom::XusbGameControllerProvider const& provider)
            : CustomDevice(provider)
        {
        }

        void GetLatestInput(uint64_t& timestamp, uint8_t& reportId, winrt::com_array<uint8_t>& reportBuffer);
        void SetVibration(double lowFrequencyMotorSpeed, double highFrequencyMotorSpeed);

        void OnInputReceived(uint64_t timestamp, uint8_t reportId, winrt::array_view<uint8_t const> reportBuffer);

        winrt::hstring GetRuntimeClassName() const
        {
            return L"WGIC.XusbDevice";
        }
    };
}

namespace winrt::WGIC::factory_implementation
{
    struct XusbDevice : XusbDeviceT<XusbDevice, implementation::XusbDevice>
    {
    };
}
