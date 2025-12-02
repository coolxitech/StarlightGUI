#include "pch.h"
#include "Process_ModulePage.xaml.h"
#if __has_include("Process_ModulePage.g.cpp")
#include "Process_ModulePage.g.cpp"
#endif


#include <winrt/Microsoft.UI.Xaml.h>
#include <winrt/Microsoft.UI.Xaml.Media.Imaging.h>
#include <winrt/Windows.Storage.Streams.h>
#include <winrt/Windows.System.h>
#include <winrt/Windows.UI.h>
#include <winrt/Windows.UI.Core.h>
#include <winrt/Windows.Graphics.Imaging.h>
#include <winrt/Windows.Foundation.h>
#include <TlHelp32.h>
#include <Psapi.h>
#include <sstream>
#include <iomanip>
#include <Utils/Utils.h>
#include <Utils/TaskUtils.h>
#include <Utils/KernelBase.h>
#include <InfoWindow.xaml.h>
#include <MainWindow.xaml.h>

using namespace winrt;
using namespace Microsoft::UI::Text;
using namespace Microsoft::UI::Xaml;
using namespace Microsoft::UI::Xaml::Controls;
using namespace Microsoft::UI::Xaml::Media;
using namespace Microsoft::UI::Xaml::Media::Imaging;
using namespace Windows::Storage::Streams;
using namespace Windows::Graphics::Imaging;
using namespace Windows::System;

namespace winrt::StarlightGUI::implementation
{
    static int safeAcceptedPID = -1;

    Process_ModulePage::Process_ModulePage() {
        InitializeComponent();

        ModuleListView().ItemsSource(m_moduleList);
        ModuleListView().ItemContainerTransitions().Clear();
        ModuleListView().ItemContainerTransitions().Append(EntranceThemeTransition());

        this->Loaded([this](auto&&, auto&&) {
            LoadModuleList();
            });
    }

    void Process_ModulePage::ModuleListView_RightTapped(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
    {
        auto listView = sender.as<ListView>();

        if (auto fe = e.OriginalSource().try_as<FrameworkElement>())
        {
            auto container = FindParent<ListViewItem>(fe);
            if (container)
            {
                listView.SelectedItem(container.Content());
            }
        }

        if (!listView.SelectedItem()) return;

        auto item = listView.SelectedItem().as<winrt::StarlightGUI::MokuaiInfo>();

        MenuFlyout menuFlyout;

        MenuFlyoutItem itemRefresh;
        itemRefresh.Icon(CreateFontIcon(L"\ue72c"));
        itemRefresh.Text(L"刷新");
        itemRefresh.Click([this](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            LoadModuleList();
            co_return;
            });

        MenuFlyoutSeparator separatorR;

        // 选项1.1
        MenuFlyoutSubItem item1_1;
        item1_1.Icon(CreateFontIcon(L"\ue8c8"));
        item1_1.Text(L"复制信息");
        MenuFlyoutItem item1_1_sub1;
        item1_1_sub1.Icon(CreateFontIcon(L"\ue943"));
        item1_1_sub1.Text(L"名称");
        item1_1_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(item.Name().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item1_1.Items().Append(item1_1_sub1);
        MenuFlyoutItem item1_1_sub2;
        item1_1_sub2.Icon(CreateFontIcon(L"\uec6c"));
        item1_1_sub2.Text(L"路径");
        item1_1_sub2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(item.Path().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item1_1.Items().Append(item1_1_sub2);
        MenuFlyoutItem item1_1_sub3;
        item1_1_sub3.Icon(CreateFontIcon(L"\ueb1d"));
        item1_1_sub3.Text(L"地址");
        item1_1_sub3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(item.Address().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item1_1.Items().Append(item1_1_sub3);

        menuFlyout.Items().Append(itemRefresh);
        menuFlyout.Items().Append(separatorR);
        menuFlyout.Items().Append(item1_1);

        menuFlyout.ShowAt(listView, e.GetPosition(listView));
    }

    winrt::Windows::Foundation::IAsyncAction Process_ModulePage::LoadModuleList()
    {
        if (!processForInfoWindow) co_return;

        auto start = std::chrono::high_resolution_clock::now();

        auto lifetime = get_strong();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::MokuaiInfo> modules;
        modules.reserve(500);

        // 获取句柄列表
        try {
            KernelInstance::EnumProcessModule(processForInfoWindow.EProcessULong(), modules);
        }
        catch (...) {

        }

        co_await wil::resume_foreground(DispatcherQueue());

        if (modules.size() >= 1000) {
            CreateInfoBarAndDisplay(L"警告", L"该进程持有过多模块，程序无法完整显示，将显示前1000条！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
        }

        m_moduleList.Clear();
        for (const auto& module : modules) {
            if (module.Name().empty()) module.Name(L"(未知)");
            if (module.Path().empty()) module.Path(L"(未知)");

            m_moduleList.Append(module);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 更新模块数量文本
        std::wstringstream countText;
        countText << L"共 " << m_moduleList.Size() << L" 个模块 (" << duration.count() << " ms)";
        ModuleCountText().Text(countText.str());
        modules.clear();
    }

    template <typename T>
    T Process_ModulePage::FindParent(DependencyObject const& child)
    {
        DependencyObject parent = VisualTreeHelper::GetParent(child);
        while (parent && !parent.try_as<T>())
        {
            parent = VisualTreeHelper::GetParent(parent);
        }
        return parent.try_as<T>();
    }
}