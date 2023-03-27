#pragma once
#include "pch.h"
#include "WGIC.HidDevice.g.h"
#include "WGIC/CustomDevice.h"

namespace winrt::WGIC::implementation
{
    struct HidDevice : WGIC::CustomDevice<HidDevice, WGIC::HidDevice, Custom::HidGameControllerProvider,
        Custom::IHidGameControllerInputSink>
    {
    public:
        static void RegisterHardwareIds(uint16_t vendorId, uint16_t productId);

    private:
        uint64_t m_currentTimestamp = 0;
        uint8_t m_currentReportId = 0;
        winrt::com_array<uint8_t> m_currentReport;

    public:
        HidDevice(Custom::HidGameControllerProvider const& provider)
            : CustomDevice(provider)
        {
        }

        void GetLatestReport(uint64_t& timestamp, uint8_t& reportId, winrt::com_array<uint8_t>& reportBuffer);
        void SendOutputReport(uint8_t& reportId, winrt::array_view<uint8_t const> reportBuffer);
        void SendFeatureReport(uint8_t& reportId, winrt::array_view<uint8_t const> reportBuffer);

        void OnInputReportReceived(uint64_t timestamp, uint8_t reportId, winrt::array_view<uint8_t const> reportBuffer);

        winrt::hstring GetRuntimeClassName() const
        {
            return L"WGIC.HidDevice";
        }
    };
}

namespace winrt::WGIC::factory_implementation
{
    struct HidDevice : HidDeviceT<HidDevice, implementation::HidDevice>
    {
    };
}
