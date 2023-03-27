#include "pch.h"
#include "WGIC/GipDevice.h"
#include "WGIC.GipDevice.g.cpp"
#include "WGIC/DeviceFactory.h"

namespace winrt::WGIC::implementation
{
    void GipDevice::RegisterInterfaceGuid(winrt::guid interfaceGuid)
    {
        DeviceFactory::RegisterGipInterfaceGuid(interfaceGuid);
    }

    void GipDevice::GetLatestMessage(uint64_t& timestamp, Custom::GipMessageClass& messageClass, uint8_t& messageId,
        uint8_t& sequenceId, winrt::com_array<uint8_t>& messageBuffer)
    {
        std::lock_guard<std::mutex> lock(m_readingLock);
        if (m_inputSuspended)
        {
            timestamp = 0;
            messageClass = static_cast<Custom::GipMessageClass>(0);
            messageId = 0;
            sequenceId = 0;
            messageBuffer = winrt::com_array<uint8_t>(0);
        }
        else
        {
            timestamp = m_currentTimestamp;
            messageId = m_currentMessageId;
            sequenceId = m_currentMessageSequence;
            int size = m_currentMessage.size();
            messageBuffer = winrt::com_array<uint8_t>(size);
            memcpy(messageBuffer.data(), m_currentMessage.data(), size);
        }
    }

    void GipDevice::SendMessage(Custom::GipMessageClass const& messageClass, uint8_t messageId,
        winrt::array_view<uint8_t const> messageBuffer)
    {
        m_provider.SendMessage(messageClass, messageId, messageBuffer);
    }

    void GipDevice::OnKeyReceived(uint64_t timestamp, uint8_t keyCode, bool isPressed)
    {
#ifdef _DEBUG
        auto logger = spdlog::get(s_loggerName)->clone("GipDevice::OnKeyReceived");
        logger->debug("Keycode 0x{:02X} {} at {}",
            keyCode,
            isPressed ? "pressed" : "released",
            timestamp
        );
#endif

        std::lock_guard<std::mutex> lock(m_readingLock);
        m_currentTimestamp = timestamp;
        m_currentMessageClass = Custom::GipMessageClass::Command;
        m_currentMessageId = 0x07;
        m_currentMessageSequence = 0;
        m_currentMessage = winrt::com_array<uint8_t>(2);
        m_currentMessage[0] = isPressed ? 0x01 : 0x00;
        m_currentMessage[1] = keyCode;
    }

    void GipDevice::OnMessageReceived(uint64_t timestamp, Custom::GipMessageClass const& messageClass,
        uint8_t messageId, uint8_t sequenceId, winrt::array_view<uint8_t const> messageBuffer)
    {
#ifdef _DEBUG
        std::string messageClassName;
        switch (messageClass)
        {
        case Custom::GipMessageClass::Command: messageClassName = "Command"; break;
        case Custom::GipMessageClass::LowLatency: messageClassName = "LowLatency"; break;
        case Custom::GipMessageClass::StandardLatency: messageClassName = "StandardLatency"; break;
        default: messageClassName = std::to_string((uint32_t)messageClass); break;
        }

        auto logger = spdlog::get(s_loggerName)->clone("GipDevice::OnMessageReceived");
        logger->debug("Class: {}, ID: 0x{:02X}, sequence: {}, length: {}",
            messageClassName,
            messageId,
            sequenceId,
            messageBuffer.size()
        );
#endif

        std::lock_guard<std::mutex> lock(m_readingLock);
        m_currentTimestamp = timestamp;
        m_currentMessageClass = messageClass;
        m_currentMessageId = messageId;
        m_currentMessageSequence = sequenceId;
        int size = messageBuffer.size();
        m_currentMessage = winrt::com_array<uint8_t>(size);
        memcpy(m_currentMessage.data(), messageBuffer.data(), size);
    }
}