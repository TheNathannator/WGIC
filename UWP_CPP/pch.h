#pragma once

#include <windows.h>
#include <unknwn.h>
#include <restrictederrorinfo.h>
#include <hstring.h>

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.ApplicationModel.Activation.h>
#include <winrt/Windows.Gaming.Input.h>
#include <winrt/Windows.Gaming.Input.Custom.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.UI.Xaml.h>
#include <winrt/Windows.UI.Xaml.Controls.h>
#include <winrt/Windows.UI.Xaml.Controls.Primitives.h>
#include <winrt/Windows.UI.Xaml.Data.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Windows.UI.Xaml.Markup.h>
#include <winrt/Windows.UI.Xaml.Navigation.h>
#include <winrt/UWP_CPP.h>
#include <winrt/WGIC.h>

namespace Foundation = winrt::Windows::Foundation;
namespace Collections = winrt::Windows::Foundation::Collections;

namespace ApplicationModel = winrt::Windows::ApplicationModel;
namespace Activation = winrt::Windows::ApplicationModel::Activation;
namespace System = winrt::Windows::System;
namespace UI = winrt::Windows::UI;
namespace Xaml = winrt::Windows::UI::Xaml;

namespace Input = winrt::Windows::Gaming::Input;
namespace Custom = winrt::Windows::Gaming::Input::Custom;

namespace TestApp = winrt::UWP_CPP;
namespace WGIC = winrt::WGIC;

#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <fmt/core.h>
#include <fmt/xchar.h>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/bin_to_hex.h>
#include <spdlog/sinks/msvc_sink.h>
constexpr char* s_loggerName = "UWP_CPP";

#include "Utilities.h"
