#include "pch.h"
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
using namespace WinUI3Package;
using namespace Windows::UI;
using namespace Windows::Graphics;
using namespace Windows::Storage;
using namespace Windows::Storage::Streams;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Microsoft::UI::Dispatching;
using namespace Microsoft::UI::Composition::SystemBackdrops;

namespace winrt::StarlightGUI::implementation
{
    MainWindow* g_mainWindowInstance = nullptr;
    static HWND globalHWND;

    MainWindow::MainWindow()
    {
        InitializeComponent();

        auto windowNative{ this->try_as<::IWindowNative>() };
        HWND hWnd{ 0 };
        windowNative->get_WindowHandle(&hWnd);
        globalHWND = hWnd;

        this->ExtendsContentIntoTitleBar(true);
        this->SetTitleBar(AppTitleBar());
        this->AppWindow().TitleBar().PreferredHeightOption(winrt::Microsoft::UI::Windowing::TitleBarHeightOption::Tall);

        int32_t width = ReadConfig("window_width", 1200);
        int32_t height = ReadConfig("window_height", 800);

        this->AppWindow().Resize(SizeInt32{ width, height });

        // 外观
        LoadBackdrop();
        LoadBackground();
        LoadNavigation();

        g_mainWindowInstance = this;

        // Home page
        MainFrame().Navigate(xaml_typename<StarlightGUI::HomePage>());
        RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(0));

        Closed([this](auto&& sender, const winrt::Microsoft::UI::Xaml::WindowEventArgs& args) {
            int32_t width = this->AppWindow().Size().Width;
            int32_t height = this->AppWindow().Size().Height;

            SaveConfig("window_width", width);
            SaveConfig("window_height", height);

            LOG_INFO(L"MainWindow", L"Saved window size.");
            LOGGER_CLOSE();
            });

        LOG_INFO(L"MainWindow", L"MainWindow initialized.");
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
        else if (invokedItem == L"内核模块") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::KernelModulePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(2));
        }
        else if (invokedItem == L"文件管理") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::FilePage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(3));
        }
        else if (invokedItem == L"系统功能") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::UtilityPage>());
            RootNavigation().SelectedItem(RootNavigation().MenuItems().GetAt(4));
        }
        else if (invokedItem == L"关于") {
            MainFrame().Navigate(xaml_typename<StarlightGUI::HelpPage>());
            RootNavigation().SelectedItem(RootNavigation().FooterMenuItems().GetAt(0));
        }
    }

    winrt::fire_and_forget MainWindow::LoadBackdrop()
    {
        std::string background_type = ReadConfig("background_type", "Static");
        std::string option = "*";

        if (background_type == "Mica") {
            CustomMicaBackdrop micaBackdrop = CustomMicaBackdrop();

            this->SystemBackdrop(micaBackdrop);

            option = ReadConfig("mica_type", "BaseAlt");
            if (option == "Base") {
                micaBackdrop.Kind(MicaKind::Base);
            }
            else {
                micaBackdrop.Kind(MicaKind::BaseAlt);
            }
        }
        else if (background_type == "Acrylic") {
            CustomAcrylicBackdrop acrylicBackdrop = CustomAcrylicBackdrop();

            this->SystemBackdrop(acrylicBackdrop);

            option = ReadConfig("acrylic_type", "Default");
            if (option == "Base") {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Base);
            }
            else if (option == "Thin") {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Thin);
            }
            else {
                acrylicBackdrop.Kind(DesktopAcrylicKind::Default);
            }
        }
        else
        {
            this->SystemBackdrop(nullptr);
        }

        LOG_INFO(L"MainWindow", L"Loaded backdrop async with options: [%s, %s]", std::wstring(background_type.begin(), background_type.end()).c_str(), std::wstring(option.begin(), option.end()).c_str());
        co_return;
    }

    winrt::fire_and_forget MainWindow::LoadBackground()
    {
        std::string background_image = ReadConfig("background_image", "");

        HANDLE hFile = CreateFileA(background_image.c_str(), GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hFile != INVALID_HANDLE_VALUE) {
            CloseHandle(hFile);

            try {
                StorageFile file = co_await StorageFile::GetFileFromPathAsync(to_hstring(background_image));

                if (file && file.IsAvailable() && (file.FileType() == L".png" || file.FileType() == L".jpg" || file.FileType() == L".bmp" || file.FileType() == L".jpeg")) {
                    ImageBrush brush;
                    BitmapImage bitmapImage;
                    auto& stream = co_await file.OpenReadAsync();
                    bitmapImage.SetSource(stream);
                    brush.ImageSource(bitmapImage);

                    int opacity = ReadConfig("image_opacity", 20);
                    std::string stretch = ReadConfig("image_stretch", "UniformToFill");

                    brush.Stretch(stretch == "None" ? Stretch::None : stretch == "Uniform" ? Stretch::Uniform : stretch == "Fill" ? Stretch::Fill : Stretch::UniformToFill);
                    brush.Opacity(opacity / 100.0);

                    MainWindowGrid().Background(brush);

                    LOG_INFO(L"MainWindow", L"Loaded background async with options: [%s, %d, %s]", to_hstring(background_image).c_str(), opacity, to_hstring(stretch).c_str());
                }
            }
            catch (hresult_error) {
                SolidColorBrush brush;
                brush.Color(Colors::Transparent());

                MainWindowGrid().Background(brush);
                LOG_ERROR(L"MainWindow", L"Unable to load window backgroud! Applying transparent brush instead.");
            }
        }
        else {
            SolidColorBrush brush;
            brush.Color(Colors::Transparent());

            MainWindowGrid().Background(brush);
            LOG_ERROR(L"MainWindow", L"Background file does not exist. Applying transparent brush instead.");
        }
        co_return;
    }

    winrt::fire_and_forget MainWindow::LoadNavigation()
    {
        std::string navigation_style = ReadConfig("navigation_style", "LeftCompact");

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

        LOG_INFO(L"MainWindow", L"Loaded navigation async with options: [%s]", to_hstring(navigation_style).c_str());
        co_return;
    }

    HWND MainWindow::GetWindowHandle()
    {
        return globalHWND;
    }
}