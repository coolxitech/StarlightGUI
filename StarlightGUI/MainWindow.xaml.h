#pragma once

#include "MainWindow.g.h"
#include <winrt/Microsoft.UI.Composition.SystemBackdrops.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>

namespace winrt::StarlightGUI::implementation
{
    extern winrt::Microsoft::UI::Xaml::Media::MicaBackdrop micaBackdrop;
    extern winrt::Microsoft::UI::Xaml::Media::DesktopAcrylicBackdrop acrylicBackdrop;

    struct MainWindow : MainWindowT<MainWindow>
    {
        MainWindow();
        ~MainWindow();

        void MinimizeButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MaximizeButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CloseButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void UpdateMaximizeButton();

        void RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args);

        HWND GetWindowHandle();

        // 背景
        void LoadBackdrop();

        // 窗口消息处理
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
        void HandleWindowPosChanged();

        winrt::Microsoft::UI::Xaml::Controls::TextBlock MaximizeButtonContent() {
            return MaximizeButton().Content().as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>();
        }

        std::vector<winrt::StarlightGUI::InfoWindow> m_openWindows;

    };

    extern MainWindow* g_mainWindowInstance;
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct MainWindow : MainWindowT<MainWindow, implementation::MainWindow>
    {
    };
}