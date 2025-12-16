#include "pch.h"
#include "HomePage.xaml.h"
#if __has_include("HomePage.g.cpp")
#include "HomePage.g.cpp"
#endif

#include <winrt/Windows.System.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.Web.Http.h>
#include <winrt/Windows.Data.Json.h>
#include <winrt/Windows.System.UserProfile.h>
#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Globalization.h>
#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Microsoft.UI.h>
#include <winrt/Microsoft.UI.Windowing.h>
#include <winrt/Microsoft.UI.Dispatching.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Microsoft.UI.Xaml.Controls.h>
#include <random>
#include <chrono>
#include "MainWindow.xaml.h"
#include "UpdateDialog.xaml.h"

using namespace winrt;
using namespace Windows::Web::Http;
using namespace Windows::Data::Json;
using namespace Windows::System;
using namespace Windows::Storage;
using namespace Windows::Foundation;
using namespace Windows::Globalization;
using namespace Windows::ApplicationModel;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;

namespace winrt::StarlightGUI::implementation
{
    HomePage::HomePage()
    {
        InitializeComponent();

        SetGreetingText();
        SetUserProfile();
        try {
            FetchHitokoto();
        }
        catch (hresult_error) {
            hitokoto = L"无法加载内容... :(";
        }
        SetupClock();

        if (!loaded) {
            this->Loaded([this](auto&&, auto&&) {
                if (!KernelInstance::IsRunningAsAdmin()) {
                    CreateInfoBarAndDisplay(L"警告", L"当前正以常规模式运行，大部分功能将无法使用或功能残缺。欲使用完整功能请以管理员身份运行！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                }
                else {
                    CreateInfoBarAndDisplay(L"信息", L"正在加载驱动，这可能需要一点时间...", InfoBarSeverity::Informational, XamlRoot(), InfoBarPanel());
                    LoadDriverPath();
                }
                try {
                    CheckUpdate();
                }
                catch (hresult_error) {
                    CreateInfoBarAndDisplay(L"警告", L"检查更新失败！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                }
                loaded = true;
                
                });
        }

        LOG_INFO(L"HomePage", L"HomePage initialized.");
    }

    void HomePage::SetGreetingText()
    {
        if (greeting.empty()) {
            std::vector<hstring> greetings = {
                L"欢迎回来",
                L"你好",
                L"Hi",
                L"TimeFormat",
            };

            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_int_distribution<> dis(0, greetings.size() - 1);
            greeting = greetings[dis(gen)];

            DateTime currentDateTime;
            Calendar calendar;
            calendar.SetToNow();
            int currentHour = calendar.Hour();

            if (greeting == L"TimeFormat") {
                if (currentHour < 12)
                {
                    greeting = L"上午好";
                }
                else if (currentHour < 18)
                {
                    greeting = L"下午好";
                }
                else if (currentHour >= 18)
                {
                    greeting = L"晚上好";
                }
            }
        }

        AppIntroduction().Text(L"欢迎使用 Starlight GUI！");
    }

    winrt::fire_and_forget HomePage::SetUserProfile()
    {
        auto weak_this = get_weak();

        if (username.empty() || avatar == nullptr) {
            co_await winrt::resume_background();

            auto users = co_await User::FindAllAsync(UserType::LocalUser, UserAuthenticationStatus::LocallyAuthenticated);

            if (users != nullptr && users.Size() > 0)
            {
                auto user = users.GetAt(0);
                auto picture = co_await user.GetPictureAsync(UserPictureSize::Size64x64);

                if (picture != nullptr)
                {
                    auto stream = co_await picture.OpenReadAsync();

                    if (stream != nullptr)
                    {
                        co_await wil::resume_foreground(DispatcherQueue());

                        BitmapImage bitmapImage;
                        co_await bitmapImage.SetSourceAsync(stream);
                        avatar = bitmapImage;

                        LOG_INFO(__WFUNCTION__, L"Got user account picture.");
                    }
                }

                co_await winrt::resume_background();

                auto displayName = co_await user.GetPropertyAsync(KnownUserProperties::DisplayName());

                if (displayName != nullptr && !displayName.as<winrt::hstring>().empty())
                {
                    username = displayName.as<winrt::hstring>();
                    LOG_INFO(__WFUNCTION__, L"Got user account name.");
                }
            }
        }


        if (auto strong_this = weak_this.get()) {
            co_await wil::resume_foreground(DispatcherQueue());
            UserAvatar().ImageSource(avatar.as<winrt::Microsoft::UI::Xaml::Media::ImageSource>());
            WelcomeText().Text(greeting + L", " + username + L"！");
        }
    }

    winrt::fire_and_forget HomePage::FetchHitokoto()
    {
        auto weak_this = get_weak();

        if (hitokoto.empty()) {
            co_await winrt::resume_background();

            // Async request to get hitokoto text
            HttpClient client;
            Uri uri(L"https://v1.hitokoto.cn/?c=a&c=b&c=c&c=d&c=i&c=k");

            LOG_INFO(__WFUNCTION__, L"Sending hitokoto request...");
            hstring result = co_await client.GetStringAsync(uri);

            // Read json object
            auto json = Windows::Data::Json::JsonObject::Parse(result);
            hitokoto = L"“" + json.GetNamedString(L"hitokoto") + L"”";

            LOG_INFO(__WFUNCTION__, L"Hitokoto result: %s", hitokoto.c_str());
        }

        if (auto strong_this = weak_this.get()) {
            co_await wil::resume_foreground(DispatcherQueue());
            HitokotoText().Text(hitokoto);
        }

        co_return;
    }

    winrt::fire_and_forget HomePage::CheckUpdate()
    {
        if (!ReadConfig("check_update", true)) co_return;

        auto weak_this = get_weak();

        int currentBuildNumber = unbox_value<int>(Application::Current().Resources().TryLookup(box_value(L"BuildNumber")));
        int latestBuildNumber = 0;

        co_await winrt::resume_background();

        HttpClient client;
        Uri uri(L"https://pastebin.com/raw/kz5qViYF");

        LOG_INFO(L"Updater", L"Sending update check request...");
        hstring result = co_await client.GetStringAsync(uri);

        auto json = Windows::Data::Json::JsonObject::Parse(result);
        latestBuildNumber = json.GetNamedNumber(L"build_number");

        if (auto strong_this = weak_this.get()) {
            co_await wil::resume_foreground(DispatcherQueue());

            LOG_INFO(L"Updater", L"Current: %d, Latest: %d", currentBuildNumber, latestBuildNumber);

            if (latestBuildNumber == 0) {
                LOG_WARNING(L"Updater", L"Latest = 0, check failed.");
                CreateInfoBarAndDisplay(L"警告", L"检查更新失败！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
            }
            else if (latestBuildNumber == currentBuildNumber) {
                LOG_INFO(L"Updater", L"Latest = current, we are on the latest version.");
                CreateInfoBarAndDisplay(L"信息", L"你正在使用最新版本的 Starlight GUI！", InfoBarSeverity::Informational, XamlRoot(), InfoBarPanel());
            }
            else if (latestBuildNumber > currentBuildNumber) {
                LOG_INFO(L"Updater", L"Latest > current, new version avaliable. Calling up update dialog.");
                auto dialog = winrt::make<winrt::StarlightGUI::implementation::UpdateDialog>();
                dialog.LatestVersion(json.GetNamedString(L"version"));
                dialog.XamlRoot(this->XamlRoot());

                auto result = co_await dialog.ShowAsync();

                if (result == ContentDialogResult::Primary) {
                    Uri target(json.GetNamedString(L"download_link"));
                    auto result = co_await Launcher::LaunchUriAsync(target);

                    if (result) {
                        CreateInfoBarAndDisplay(L"成功", L"已在浏览器打开网页！", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    }
                    else {
                        CreateInfoBarAndDisplay(L"失败", L"无法打开网页！", InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
                    }
                }
            }
            else if (latestBuildNumber < currentBuildNumber) {
                LOG_INFO(L"DriverUtils", L"Latest < current, maybe we are on a dev environment.", kernelPath.c_str());
                CreateInfoBarAndDisplay(L"信息", L"你正在使用 Starlight GUI 的开发版本！", InfoBarSeverity::Informational, XamlRoot(), InfoBarPanel());
            }
        }

        co_return;
    }

    void HomePage::SetupClock()
    {
        // Tick every sec
        m_clockTimer = DispatcherQueue().CreateTimer();
        m_clockTimer.Interval(std::chrono::seconds(1));
        m_clockTimer.Tick({ this, &HomePage::OnClockTick });
        m_clockTimer.Start();

        UpdateClock();
    }

    void HomePage::OnClockTick(IInspectable const& sender, IInspectable const&)
    {
        UpdateClock();
    }

    void HomePage::UpdateClock()
    {
        Calendar calendar;
        calendar.SetToNow();

        int hour = calendar.Hour();
        int minute = calendar.Minute();
        int second = calendar.Second();

        auto splitDigits = [](int value) -> std::pair<hstring, hstring> {
            int digit1 = value / 10;  // 十位
            int digit2 = value % 10;  // 个位
            return { to_hstring(digit1), to_hstring(digit2) };
            };

        auto hourDigits = splitDigits(hour);
        Hour1().Text(hourDigits.first);
        Hour2().Text(hourDigits.second);

        auto minuteDigits = splitDigits(minute);
        Minute1().Text(minuteDigits.first);
        Minute2().Text(minuteDigits.second);

        auto secondDigits = splitDigits(second);
        Second1().Text(secondDigits.first);
        Second2().Text(secondDigits.second);
    }

    winrt::fire_and_forget HomePage::LoadDriverPath() {
        auto strong = get_strong();

        g_mainWindowInstance->RootNavigation().IsEnabled(false);
        LOG_INFO(L"DriverUtils", L"Loading necessary drivers...");
        if (kernelPath.empty() || astralPath.empty()) {
            try {
                co_await winrt::resume_background();

                auto& appFolder = Package::Current().InstalledLocation();
                auto& assetsFolder = co_await appFolder.GetFolderAsync(L"Assets");
                auto& kernelFile = co_await assetsFolder.GetFileAsync(L"kernel.sys");
                auto& astralFile = co_await assetsFolder.GetFileAsync(L"AstralX.sys");

                if (kernelFile) {
                    kernelPath = kernelFile.Path();

                    LOG_INFO(L"DriverUtils", L"Kernel.sys path [%s], load it.", kernelPath.c_str());
                    DriverUtils::LoadKernelDriver(kernelPath.c_str(), unused);

                    if (astralFile) {
                        astralPath = astralFile.Path();

                        LOG_INFO(L"DriverUtils", L"AstralX.sys path [%s], load it.", astralPath.c_str());
                        DriverUtils::LoadDriver(astralPath.c_str(), L"AstralX", unused);
                    }
                }

                LOG_SUCCESS(L"DriverUtils", L"Loaded successfully.", kernelPath.c_str());
                co_await wil::resume_foreground(DispatcherQueue());
                CreateInfoBarAndDisplay(L"成功", L"驱动加载成功！", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            catch (winrt::hresult_error e) {
                LOG_ERROR(L"DriverUtils", L"Error while loading drivers!", kernelPath.c_str());
                LOG_ERROR(L"DriverUtils", L"%s", e.message().c_str());
                CreateInfoBarAndDisplay(L"警告", L"一个或多个驱动文件未找到或无法加载，部分功能可能不可用！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
            }
        }
        g_mainWindowInstance->RootNavigation().IsEnabled(true);
    }
}