#pragma once

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "shell32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "ntdll.lib")

// Undefine GetCurrentTime macro to prevent
// conflict with Storyboard::GetCurrentTime
#undef GetCurrentTime

#include <Unknwn.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Foundation.Collections.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Microsoft.Windows.Storage.Pickers.h>
#include <winrt/Microsoft.UI.Composition.h>
#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <winrt/Microsoft.UI.Xaml.Controls.Primitives.h>
#include <winrt/Microsoft.UI.Xaml.Data.h>
#include <winrt/Microsoft.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Markup.h>
#include <winrt/Microsoft.UI.Xaml.Media.h>
#include <winrt/Microsoft.UI.Xaml.Media.Animation.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Navigation.h>
#include <winrt/Microsoft.UI.Xaml.Shapes.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Interop.h>
#include <winrt/Microsoft.UI.Text.h>
#include <winrt/Microsoft.UI.h>
#include <wil/cppwinrt_helpers.h>
#include <microsoft.ui.xaml.window.h>
#include <winrt/WinUI3Package.h>

#include <Utils/ProcessInfo.h>
#include <Utils/ThreadInfo.h>
#include <Utils/HandleInfo.h>
#include <Utils/MokuaiInfo.h>
#include <Utils/KCTInfo.h>
#include <Utils/KernelModuleInfo.h>
#include <Utils/FileInfo.h>

#include <Utils/TaskUtils.h>
#include <Utils/Utils.h>
#include <Utils/KernelBase.h>
#include <Utils/Elevator.h>
#include <Utils/Config.h>
#include <Utils/CppUtils.h>
#include <Utils/Terminator.h>

static inline winrt::hstring kernelPath = L"";
static inline winrt::hstring astralPath = L"";
static inline std::wstring unused = L"";