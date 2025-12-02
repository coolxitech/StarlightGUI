#include "pch.h"
#include "InfoWindow.xaml.h"
#if __has_include("InfoWindow.g.cpp")
#include "InfoWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.UI.h>
#include <Windows.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <Utils/Config.h>
#include <MainWindow.xaml.h>
#include <Utils/ProcessInfo.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Composition::SystemBackdrops;

namespace winrt::StarlightGUI::implementation
{
    static HWND globalHWND;
    InfoWindow* g_infoWindowInstance = nullptr;
    winrt::StarlightGUI::ProcessInfo processForInfoWindow = nullptr;

    InfoWindow::InfoWindow() {
        InitializeComponent();

        auto windowNative{ this->try_as<::IWindowNative>() };
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);
        globalHWND = hWnd;

        SetWindowPos(hWnd, g_mainWindowInstance->GetWindowHandle(), 0, 0, 1200, 800, SWP_NOMOVE);

        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(AppTitleBar());

        // Background
        LoadBackdrop();

        g_infoWindowInstance = this;
        SetWindowSubclass(hWnd, &InfoWindow::WindowProc, 0, 0);

        UpdateMaximizeButton();

        for (auto& window : g_mainWindowInstance->m_openWindows) {
            if (window) {
                window.Close();
            }
        }
        g_mainWindowInstance->m_openWindows.push_back(*this);

        MainFrame().Navigate(xaml_typename<StarlightGUI::Process_ThreadPage>());
        RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
        ProcessName().Text(processForInfoWindow.Name());
    }

    void InfoWindow::RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args)
    {
        auto invokedItem = args.InvokedItem().try_as<winrt::hstring>();

        if (invokedItem == L"线程")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::Process_ThreadPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
        }
        else if (invokedItem == L"句柄")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::Process_HandlePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(1));
        }
        else if (invokedItem == L"模块")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::Process_ModulePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(2));
        }
    }

    void InfoWindow::LoadBackdrop()
    {
        auto background_type = ReadConfig("background_type", "Static");

        if (background_type == "Mica") {
            micaBackdrop = winrt::Microsoft::UI::Xaml::Media::MicaBackdrop();
            micaBackdrop.Kind(MicaKind::BaseAlt);

            this->SystemBackdrop(micaBackdrop);
        }
        else if (background_type == "Acrylic") {
            acrylicBackdrop = winrt::Microsoft::UI::Xaml::Media::DesktopAcrylicBackdrop();

            this->SystemBackdrop(acrylicBackdrop);
        }
        else
        {
            this->SystemBackdrop(nullptr);
        }
    }

    void InfoWindow::MinimizeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto hWnd = GetWindowHandle();
        ShowWindow(hWnd, SW_MINIMIZE);
    }

    void InfoWindow::MaximizeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto hWnd = GetWindowHandle();

        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);
        GetWindowPlacement(hWnd, &wp);

        if (wp.showCmd == SW_SHOWMAXIMIZED)
        {
            ShowWindow(hWnd, SW_RESTORE);
        }
        else
        {
            ShowWindow(hWnd, SW_MAXIMIZE);
        }
    }

    void InfoWindow::CloseButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto it = std::find(g_mainWindowInstance->m_openWindows.begin(), g_mainWindowInstance->m_openWindows.end(), *this);
        if (it != g_mainWindowInstance->m_openWindows.end()) {
            g_mainWindowInstance->m_openWindows.erase(it);
        }
        this->Close();
    }

    void InfoWindow::UpdateMaximizeButton()
    {
        auto hWnd = GetWindowHandle();

        WINDOWPLACEMENT wp;
        wp.length = sizeof(WINDOWPLACEMENT);

        if (GetWindowPlacement(hWnd, &wp))
        {
            if (wp.showCmd == SW_SHOWMAXIMIZED)
            {
                MaximizeButtonContent().Text(L"\uE923"); // 恢复
            }
            else
            {
                MaximizeButtonContent().Text(L"\uE922"); // 最大化
            }
        }
    }

    HWND InfoWindow::GetWindowHandle()
    {
        return globalHWND;
    }

    LRESULT CALLBACK InfoWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        switch (message)
        {
        case WM_WINDOWPOSCHANGED:
            if (g_infoWindowInstance)
            {
                g_infoWindowInstance->HandleWindowPosChanged();
            }
            break;
        case WM_NCDESTROY:
            RemoveWindowSubclass(hWnd, &InfoWindow::WindowProc, uIdSubclass);
            break;
        }


        return DefSubclassProc(hWnd, message, wParam, lParam);
    }

    void InfoWindow::HandleWindowPosChanged()
    {

        auto dispatcherQueue = this->DispatcherQueue();
        if (dispatcherQueue)
        {
            dispatcherQueue.TryEnqueue(DispatcherQueuePriority::Normal, [this]() {
                UpdateMaximizeButton();
                });
        }
    }
}
