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
        this->AppWindow().TitleBar().PreferredHeightOption(winrt::Microsoft::UI::Windowing::TitleBarHeightOption::Tall);

        // 外观
        LoadBackdrop();
        LoadNavigation();

        g_mainWindowInstance = this;

        // Home page
        MainFrame().Navigate(xaml_typename<StarlightGUI::HomePage>());
        RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));
    }

    MainWindow::~MainWindow()
    {
        for (auto& window : m_openWindows) {
            if (window) {
                window.Close();
            }
        }
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

    void MainWindow::LoadNavigation()
    {
        auto navigation_style = ReadConfig("navigation_style", "LeftCompact");

        if (navigation_style == "Left") {
            RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::Left);
        }
        else if (navigation_style == "Top") {
            RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::Top);
        }
        else
        {
            RootNavigation().PaneDisplayMode(NavigationViewPaneDisplayMode::LeftCompact);
        }
    }

    HWND MainWindow::GetWindowHandle()
    {
        return globalHWND;
    }
}