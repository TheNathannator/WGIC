#include "pch.h"
#include "WGIC/HidDevice.h"
#include "WGIC.HidDevice.g.cpp"
#include "WGIC/DeviceFactory.h"

namespace winrt::WGIC::implementation
{
    void HidDevice::RegisterHardwareIds(uint16_t vendorId, uint16_t productId)
    {
        DeviceFactory::RegisterHardwareIds(vendorId, productId);
    }

    void HidDevice::GetLatestReport(uint64_t& timestamp, uint8_t& reportId, winrt::com_array<uint8_t>& reportBuffer)
    {
        std::lock_guard<std::mutex> lock(m_readingLock);
        if (m_inputSuspended)
        {
            timestamp = 0;
            reportId = 0;
            reportBuffer = winrt::com_array<uint8_t>(0);
        }
        else
        {
            timestamp = m_currentTimestamp;
            reportId = m_currentReportId;
            int size = m_currentReport.size();
            reportBuffer = winrt::com_array<uint8_t>(size);
            memcpy(reportBuffer.data(), m_currentReport.data(), size);
        }
    }

    void HidDevice::SendOutputReport(uint8_t& reportId, winrt::array_view<uint8_t const> reportBuffer)
    {
        m_provider.SendOutputReport(reportId, reportBuffer);
    }

    void HidDevice::SendFeatureReport(uint8_t& reportId, winrt::array_view<uint8_t const> reportBuffer)
    {
        m_provider.SendFeatureReport(reportId, reportBuffer);
    }

    void HidDevice::OnInputReportReceived(uint64_t timestamp, uint8_t reportId, winrt::array_view<uint8_t const> reportBuffer)
    {
#ifdef _DEBUG
        auto logger = spdlog::get(s_loggerName)->clone("HidDevice::OnInputReportReceived");
        logger->debug("ID: 0x{:02X}, length: {}",
            reportId,
            reportBuffer.size()
        );
#endif

        std::lock_guard<std::mutex> lock(m_readingLock);
        m_currentTimestamp = timestamp;
        m_currentReportId = reportId;
        int size = reportBuffer.size();
        m_currentReport = winrt::com_array<uint8_t>(size);
        memcpy(m_currentReport.data(), reportBuffer.data(), size);
    }
}