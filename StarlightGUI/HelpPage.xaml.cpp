#include "pch.h"
#include "HelpPage.xaml.h"
#if __has_include("HelpPage.g.cpp")
#include "HelpPage.g.cpp"
#endif

using namespace winrt;
using namespace Windows::System;
using namespace Windows::Foundation;
using namespace Microsoft::UI::Xaml;

namespace winrt::StarlightGUI::implementation
{
    HelpPage::HelpPage() {
        InitializeComponent();

        LOG_INFO(L"HelpPage", L"HelpPage initialized.");
    }

    void HelpPage::GithubButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://github.com/RinoRika/StarlightGUI");
    }

    void HelpPage::Github2Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://github.com/HO-COOH/WinUIEssentials");
    }

    void HelpPage::GithubUserButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://github.com/RinoRika");
    }

    void HelpPage::GithubUser2Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://github.com/HO-COOH");
    }

    void HelpPage::GithubUser3Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://github.com/PspExitThread");
    }

    void HelpPage::GithubUser4Button_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://github.com/MuLin4396");
    }

    void HelpPage::BilibiliButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://space.bilibili.com/670866766");
    }

    void HelpPage::SponsorButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://afdian.com/a/StarsAzusa");
    }

    void HelpPage::WinUIButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
        LaunchURI(L"https://aka.ms/windev");
    }

    winrt::fire_and_forget HelpPage::LaunchURI(hstring uri) {
        Uri target(uri);
        LOG_INFO(__WFUNCTION__, L"Launching URI link: %s", uri.c_str());
        auto result = co_await Launcher::LaunchUriAsync(target);

        if (result) {
            CreateInfoBarAndDisplay(L"成功", L"已在浏览器打开网页！", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
        }
        else {
            CreateInfoBarAndDisplay(L"失败", L"无法打开网页！", InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
        }
    }
}
