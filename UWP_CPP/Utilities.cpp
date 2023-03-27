#include "pch.h"
#include "Utilities.h"

namespace Utilities
{
    const wchar_t s_NumberToCharacter[] = L"0123456789ABCDEF";
    constexpr int hexCharCount = 3; // 2 for the hex digits, one for the hyphen afterwards
    constexpr size_t maxBufferLength = SIZE_MAX / hexCharCount / sizeof(wchar_t);
    std::wstring BytesToHex(const uint8_t data[], const size_t length)
    {
        if (!data || length < 1 || length >= maxBufferLength)
            return L"";

        const size_t charCount = length * hexCharCount;
        wchar_t* buffer = static_cast<wchar_t*>(malloc(charCount * sizeof(wchar_t)));
        size_t bufferIndex = 0;
        for (size_t dataIndex = 0; dataIndex < length; dataIndex++)
        {
            uint8_t value = data[dataIndex];
            buffer[bufferIndex++] = s_NumberToCharacter[value >> 4];
            buffer[bufferIndex++] = s_NumberToCharacter[value & 0x0F];
            buffer[bufferIndex++] = L'-';
        }
        buffer[charCount - 1] = L'\0';

        std::wstring string = std::wstring(buffer, charCount);
        free(buffer);
        return string;
    }

    void LogIInspectable(std::shared_ptr<spdlog::logger> const& logger, Foundation::IInspectable const& inspectable,
        spdlog::level::level_enum level)
    {
        assert(logger);
        if (!logger)
            return;

        if (!inspectable)
        {
            logger->log(level, "Object is null!");
            return;
        }

        auto className = winrt::get_class_name(inspectable);
        auto iids = winrt::get_interfaces(inspectable);

        logger->log(level, "Class name: {}", winrt::to_string(className));
        logger->log(level, "IIDs:");
        for (auto iid : iids)
        {
            logger->log(level, "- {{{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}}}",
                iid.Data1,
                iid.Data2,
                iid.Data3,
                iid.Data4[0], iid.Data4[1],
                iid.Data4[2], iid.Data4[3], iid.Data4[4], iid.Data4[5], iid.Data4[6], iid.Data4[7]
            );
        }
    }
}
