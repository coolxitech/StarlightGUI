#pragma once

#include "InfoWindow.g.h"
#include <Utils/ProcessInfo.h>

namespace winrt::StarlightGUI::implementation
{
    struct InfoWindow : InfoWindowT<InfoWindow>
    {
        InfoWindow();

        void MinimizeButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void MaximizeButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void CloseButton_Click(winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        void UpdateMaximizeButton();

        HWND GetWindowHandle();

        void RootNavigation_ItemInvoked(Microsoft::UI::Xaml::Controls::NavigationView sender, Microsoft::UI::Xaml::Controls::NavigationViewItemInvokedEventArgs args);

        // 背景
        void LoadBackdrop();

        // 窗口消息处理
        static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData);
        void HandleWindowPosChanged();

        winrt::Microsoft::UI::Xaml::Controls::TextBlock MaximizeButtonContent() {
            return MaximizeButton().Content().as<winrt::Microsoft::UI::Xaml::Controls::TextBlock>();
        }
    };

    extern winrt::StarlightGUI::ProcessInfo processForInfoWindow;
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct InfoWindow : InfoWindowT<InfoWindow, implementation::InfoWindow>
    {
    };
}
