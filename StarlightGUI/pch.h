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

extern winrt::hstring kernelPath;
extern winrt::hstring astralPath;
extern std::wstring unused;

#include <ConsoleLogger.h>

#define __WFUNCTION__ ExtractFunctionName(__FUNCTION__)

#define LOG_DEBUG(source, message, ...)    ConsoleLogger::GetInstance().Debug(source, message, __VA_ARGS__)
#define LOG_INFO(source, message, ...)     ConsoleLogger::GetInstance().Info(source, message, __VA_ARGS__)
#define LOG_WARNING(source, message, ...)  ConsoleLogger::GetInstance().Warning(source, message, __VA_ARGS__)
#define LOG_ERROR(source, message, ...)    ConsoleLogger::GetInstance().Error(source, message, __VA_ARGS__)
#define LOG_CRITICAL(source, message, ...) ConsoleLogger::GetInstance().Critical(source, message, __VA_ARGS__)
#define LOG_SUCCESS(source, message, ...)  ConsoleLogger::GetInstance().Success(source, message, __VA_ARGS__)

#define LOGGER_INIT()			ConsoleLogger::GetInstance().Initialize()
#define LOGGER_TOGGLE()			ConsoleLogger::GetInstance().ToggleConsole()
#define LOGGER_OPEN()			ConsoleLogger::GetInstance().OpenConsole()
#define LOGGER_CLOSE()			ConsoleLogger::GetInstance().CloseConsole()
#define LOGGER_SHUTDOWN()		ConsoleLogger::GetInstance().ShutdownConsole()
#define LOGGER_CLEAR()			ConsoleLogger::GetInstance().ClearConsole()
#define LOGGER_SET_TITLE(title) ConsoleLogger::GetInstance().SetTitle(title)
#define LOGGER_SET_LEVEL(level) ConsoleLogger::GetInstance().SetMinLogLevel(level)
