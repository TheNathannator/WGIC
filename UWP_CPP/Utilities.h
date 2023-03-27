#pragma once
#include "pch.h"

namespace Utilities
{
    std::wstring BytesToHex(const uint8_t data[], const size_t length);
    void LogIInspectable(std::shared_ptr<spdlog::logger> const& logger, Foundation::IInspectable const& inspectable,
        spdlog::level::level_enum level = spdlog::level::debug);
}
