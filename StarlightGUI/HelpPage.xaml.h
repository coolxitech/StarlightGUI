#pragma once

#include "HelpPage.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct HelpPage : HelpPageT<HelpPage>
    {
        HelpPage();
        void GithubButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e); // Starlight GUI
        void Github2Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e); // WinUI Essentials
        void GithubUserButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e); // Stars (RinoRika)
        void GithubUser2Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e); // HO-COOH
        void GithubUser3Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e); // KALI_MC
        void GithubUser4Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e); // MuLin
        void BilibiliButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void SponsorButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void WinUIButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);

        IAsyncAction LaunchURI(hstring uri);
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct HelpPage : HelpPageT<HelpPage, implementation::HelpPage>
    {
    };
}
