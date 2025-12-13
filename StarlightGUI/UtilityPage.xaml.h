#pragma once

#include "UtilityPage.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct UtilityPage : UtilityPageT<UtilityPage>
    {
        UtilityPage();

        winrt::fire_and_forget FindButtonsAndDisable(DependencyObject obj);

        winrt::fire_and_forget Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget Button_Click2(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct UtilityPage : UtilityPageT<UtilityPage, implementation::UtilityPage>
    {
    };
}
