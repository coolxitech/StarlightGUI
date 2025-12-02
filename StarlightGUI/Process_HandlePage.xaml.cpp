#include "pch.h"
#include "Process_HandlePage.xaml.h"
#if __has_include("Process_HandlePage.g.cpp")
#include "Process_HandlePage.g.cpp"
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

    Process_HandlePage::Process_HandlePage() {
        InitializeComponent();

        HandleListView().ItemsSource(m_handleList);
        HandleListView().ItemContainerTransitions().Clear();
        HandleListView().ItemContainerTransitions().Append(EntranceThemeTransition());

        this->Loaded([this](auto&&, auto&&) {
            LoadHandleList();
            });
    }

    void Process_HandlePage::HandleListView_RightTapped(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
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

        auto item = listView.SelectedItem().as<winrt::StarlightGUI::HandleInfo>();

        MenuFlyout menuFlyout;

        MenuFlyoutItem itemRefresh;
        itemRefresh.Icon(CreateFontIcon(L"\ue72c"));
        itemRefresh.Text(L"刷新");
        itemRefresh.Click([this](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            LoadHandleList();
            co_return;
            });

        MenuFlyoutSeparator separatorR;

        // 选项1.1
        MenuFlyoutSubItem item1_1;
        item1_1.Icon(CreateFontIcon(L"\ue8c8"));
        item1_1.Text(L"复制信息");
        MenuFlyoutItem item1_1_sub1;
        item1_1_sub1.Icon(CreateFontIcon(L"\ue943"));
        item1_1_sub1.Text(L"类型");
        item1_1_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(item.Type().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item1_1.Items().Append(item1_1_sub1);

        menuFlyout.Items().Append(itemRefresh);
        menuFlyout.Items().Append(separatorR);
        menuFlyout.Items().Append(item1_1);

        menuFlyout.ShowAt(listView, e.GetPosition(listView));
    }

    winrt::Windows::Foundation::IAsyncAction Process_HandlePage::LoadHandleList()
    {
        if (!processForInfoWindow) co_return;

        auto start = std::chrono::high_resolution_clock::now();

        auto lifetime = get_strong();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::HandleInfo> handles;
        handles.reserve(500);

        // 获取句柄列表
        try {
            KernelInstance::EnumProcessHandle(processForInfoWindow.Id(), handles);
        } catch (...) {

        }

        co_await wil::resume_foreground(DispatcherQueue());

        if (handles.size() >= 1000) {
            CreateInfoBarAndDisplay(L"警告", L"该进程持有过多句柄，程序无法完整显示，将显示前1000条！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
        }

        m_handleList.Clear();
        for (const auto& handle : handles) {
            m_handleList.Append(handle);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 更新句柄数量文本
        std::wstringstream countText;
        countText << L"共 " << m_handleList.Size() << L" 个句柄 (" << duration.count() << " ms)";
        HandleCountText().Text(countText.str());
        handles.clear();
    }

    template <typename T>
    T Process_HandlePage::FindParent(DependencyObject const& child)
    {
        DependencyObject parent = VisualTreeHelper::GetParent(child);
        while (parent && !parent.try_as<T>())
        {
            parent = VisualTreeHelper::GetParent(parent);
        }
        return parent.try_as<T>();
    }
}