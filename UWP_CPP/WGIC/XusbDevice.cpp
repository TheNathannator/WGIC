#include "pch.h"
#include "WGIC/XusbDevice.h"
#include "WGIC.XusbDevice.g.cpp"
#include "WGIC/DeviceFactory.h"

namespace winrt::WGIC::implementation
{
    void XusbDevice::RegisterType(Custom::XusbDeviceType type, Custom::XusbDeviceSubtype subtype)
    {
        DeviceFactory::RegisterXusbType(type, subtype);
    }

    void XusbDevice::GetLatestInput(uint64_t& timestamp, uint8_t& reportId, winrt::com_array<uint8_t>& reportBuffer)
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

    void XusbDevice::SetVibration(double lowFrequencyMotorSpeed, double highFrequencyMotorSpeed)
    {
        m_provider.SetVibration(lowFrequencyMotorSpeed, highFrequencyMotorSpeed);
    }

    void XusbDevice::OnInputReceived(uint64_t timestamp, uint8_t reportId, winrt::array_view<uint8_t const> inputBuffer)
    {
#ifdef _DEBUG
        auto logger = spdlog::get(s_loggerName)->clone("XusbDevice::OnInputReceived");
        logger->debug("ID: 0x{:02X}, length: {}",
            reportId,
            inputBuffer.size()
        );
#endif

        std::lock_guard<std::mutex> lock(m_readingLock);
        m_currentTimestamp = timestamp;
        m_currentReportId = reportId;
        int size = inputBuffer.size();
        m_currentReport = winrt::com_array<uint8_t>(size);
        memcpy(m_currentReport.data(), inputBuffer.data(), size);
    }
}