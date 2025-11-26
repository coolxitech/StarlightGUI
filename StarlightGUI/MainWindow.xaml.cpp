#include "pch.h"
#include "Utils/Config.h"
#include "MainWindow.xaml.h"
#if __has_include("MainWindow.g.cpp")
#include "MainWindow.g.cpp"
#endif

#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.UI.h>
#include <Windows.h>
#include <winrt/Windows.UI.Xaml.Interop.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>

using namespace winrt;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Composition::SystemBackdrops;

namespace winrt::StarlightGUI::implementation
{
    MainWindow* g_mainWindowInstance = nullptr;
    winrt::Microsoft::UI::Xaml::Media::MicaBackdrop micaBackdrop = nullptr;
    winrt::Microsoft::UI::Xaml::Media::DesktopAcrylicBackdrop acrylicBackdrop = nullptr;
    static HWND globalHWND;

    MainWindow::MainWindow()
    {
        InitializeComponent();

        auto windowNative{ this->try_as<::IWindowNative>() };
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);
        globalHWND = hWnd;

        SetWindowPos(hWnd, NULL, 0, 0, 1200, 800, SWP_NOMOVE);

        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(AppTitleBar());

        // Background
        LoadBackdrop();

        g_mainWindowInstance = this;
        SetWindowSubclass(hWnd, &MainWindow::WindowProc, 0, 0);

        UpdateMaximizeButton();

        // Home page
        MainFrame().Navigate(xaml_typename<StarlightGUI::HomePage>());
        RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
    }

    MainWindow::~MainWindow()
    {
    }

    void MainWindow::RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args)
    {
        if (args.IsSettingsInvoked())
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::SettingsPage>());
            return;
        }

        auto invokedItem = args.InvokedItem().try_as<winrt::hstring>();

        if (invokedItem == L"主页")
        {
            MainFrame().Navigate(xaml_typename<StarlightGUI::HomePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
        }
        else if (invokedItem == L"任务管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::TaskPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(1));
        }
        else if (invokedItem == L"系统管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::ProcessPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(2));
        }
        else if (invokedItem == L"帮助") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::HelpPage>());
            RootNavigation().SelectedItem(RootNavigation().FooterMenuItems().GetAt(0));
        }
    }

    void MainWindow::LoadBackdrop()
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

    void MainWindow::MinimizeButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        auto hWnd = GetWindowHandle();
        ShowWindow(hWnd, SW_MINIMIZE);
    }

    void MainWindow::MaximizeButton_Click(IInspectable const&, RoutedEventArgs const&)
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

    void MainWindow::CloseButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        for (auto& window : m_openWindows) {
            if (window) {
                window.Close();
            }
        }
        this->Close();
    }

    void MainWindow::UpdateMaximizeButton()
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

    HWND MainWindow::GetWindowHandle()
    {
        return globalHWND;
    }

    LRESULT CALLBACK MainWindow::WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData)
    {
        switch (message)
        {
        case WM_WINDOWPOSCHANGED:
            if (g_mainWindowInstance)
            {
                g_mainWindowInstance->HandleWindowPosChanged();
            }
            break;
        case WM_NCDESTROY:
            RemoveWindowSubclass(hWnd, &MainWindow::WindowProc, uIdSubclass);
            break;
        }

        return DefSubclassProc(hWnd, message, wParam, lParam);
    }

    void MainWindow::HandleWindowPosChanged()
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