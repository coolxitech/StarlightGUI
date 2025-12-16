#include "pch.h"
#include "Utils/Config.h"
#include "App.xaml.h"
#include "MainWindow.xaml.h"

using namespace winrt;
using namespace Microsoft::UI::Xaml;


namespace winrt::StarlightGUI::implementation
{
    App::App()
    {

        UnhandledException([](winrt::Windows::Foundation::IInspectable const&,
            winrt::Microsoft::UI::Xaml::UnhandledExceptionEventArgs const& e)
            {
                winrt::hstring errorMessage = e.Message();
                LOG_CRITICAL(L"App", L"Unhandled exception detected!");
                LOG_CRITICAL(L"App", L"%s", errorMessage.c_str());
                e.Handled(true);

                if (IsDebuggerPresent()) {
                    __debugbreak();
                }
            });
    }

    void App::OnLaunched([[maybe_unused]] LaunchActivatedEventArgs const& e)
    {
        LOGGER_INIT();
        LOG_INFO(L"App", L"Launching Starlight GUI App...");
        window = make<MainWindow>();
        window.Activate();
    }
}
