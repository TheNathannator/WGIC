#pragma once
#include "pch.h"
#include "WGIC.GipDevice.g.h"
#include "WGIC/CustomDevice.h"

namespace winrt::WGIC::implementation
{
    struct GipDevice : WGIC::CustomDevice<GipDevice, WGIC::GipDevice, Custom::GipGameControllerProvider,
        Custom::IGipGameControllerInputSink>
    {
    public:
        static void RegisterInterfaceGuid(winrt::guid interfaceGuid);

    private:
        uint64_t m_currentTimestamp = 0;
        Custom::GipMessageClass m_currentMessageClass = (Custom::GipMessageClass)0;
        uint8_t m_currentMessageId = 0;
        uint8_t m_currentMessageSequence = 0;
        winrt::com_array<uint8_t> m_currentMessage;

    public:
        GipDevice(Custom::GipGameControllerProvider const& provider)
            : CustomDevice(provider)
        {
        }

        void GetLatestMessage(uint64_t& timestamp, Custom::GipMessageClass& messageClass, uint8_t& messageId,
            uint8_t& sequenceId, winrt::com_array<uint8_t>& messageBuffer);
        void SendMessage(Custom::GipMessageClass const& messageClass, uint8_t messageId,
            winrt::array_view<uint8_t const> messageBuffer);

        void OnKeyReceived(uint64_t timestamp, uint8_t keyCode, bool isPressed);
        void OnMessageReceived(uint64_t timestamp, Custom::GipMessageClass const& messageClass, uint8_t messageId,
            uint8_t sequenceId, winrt::array_view<uint8_t const> messageBuffer);

        winrt::hstring GetRuntimeClassName() const
        {
            return L"WGIC.GipDevice";
        }
    };
}

namespace winrt::WGIC::factory_implementation
{
    struct GipDevice : GipDeviceT<GipDevice, implementation::GipDevice>
    {
    };
}
