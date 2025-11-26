#include "pch.h"
#include "Process_ThreadPage.xaml.h"
#if __has_include("Process_ThreadPage.g.cpp")
#include "Process_ThreadPage.g.cpp"
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

    Process_ThreadPage::Process_ThreadPage() {
        InitializeComponent();

        ThreadListView().ItemsSource(m_threadList);
        ThreadListView().ItemContainerTransitions().Clear();
        ThreadListView().ItemContainerTransitions().Append(EntranceThemeTransition());

        this->Loaded([this](auto&&, auto&&) {
            LoadThreadList();
            });
    }

    void Process_ThreadPage::ThreadListView_RightTapped(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
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

        auto item = listView.SelectedItem().as<winrt::StarlightGUI::ThreadInfo>();

        MenuFlyout menuFlyout;

        MenuFlyoutItem itemRefresh;
        itemRefresh.Icon(CreateFontIcon(L"\ue72c"));
        itemRefresh.Text(L"刷新");
        itemRefresh.Click([this](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            LoadThreadList();
            co_return;
            });

        MenuFlyoutSeparator separatorR;

        // 选项1.1
        MenuFlyoutItem item1_1;
        item1_1.Icon(CreateFontIcon(L"\ue711"));
        item1_1.Text(L"结束线程");
        item1_1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::_TerminateThread(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功结束线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadThreadList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法结束线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 选项1.2
        MenuFlyoutItem item1_2;
        item1_2.Icon(CreateFontIcon(L"\ue8f0"));
        item1_2.Text(L"结束线程 (内核)");
        item1_2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (KernelInstance::_ZwTerminateThread(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功结束线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadThreadList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法结束线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 选项1.3
        MenuFlyoutItem item1_3;
        item1_3.Icon(CreateFontIcon(L"\ue945"));
        item1_3.Text(L"强制结束线程");
        item1_3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (safeAcceptedPID == item.Id()) {
                if (KernelInstance::MurderThread(item.Id())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功强制结束线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    LoadThreadList();
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法强制结束线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
            else {
                safeAcceptedPID = item.Id();
                CreateInfoBarAndDisplay(L"警告", L"该操作可能导致系统崩溃或进程数据丢失，如果确认继续请再次点击！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
            }
            co_return;
            });

        // 分割线1
        MenuFlyoutSeparator separator1;

        // 选项2.1
        MenuFlyoutSubItem item2_1;
        item2_1.Icon(CreateFontIcon(L"\ue912"));
        item2_1.Text(L"设置线程状态");
        MenuFlyoutItem item2_1_sub1;
        item2_1_sub1.Icon(CreateFontIcon(L"\ue769"));
        item2_1_sub1.Text(L"暂停线程");
        item2_1_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (KernelInstance::_SuspendThread(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功暂停线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadThreadList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法暂停线程: " + item.Address() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_1.Items().Append(item2_1_sub1);
        MenuFlyoutItem item2_1_sub2;
        item2_1_sub2.Icon(CreateFontIcon(L"\ue768"));
        item2_1_sub2.Text(L"恢复线程");
        item2_1_sub2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (KernelInstance::_ResumeThread(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功恢复进程: " + item.Address() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadThreadList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法恢复进程: " + item.Address() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_1.Items().Append(item2_1_sub2);
        
        menuFlyout.Items().Append(itemRefresh);
        menuFlyout.Items().Append(separatorR);
        menuFlyout.Items().Append(item1_1);
        menuFlyout.Items().Append(item1_2);
        menuFlyout.Items().Append(item1_3);
        menuFlyout.Items().Append(separator1);
        menuFlyout.Items().Append(item2_1);

        menuFlyout.ShowAt(listView, e.GetPosition(listView));
    }

    winrt::Windows::Foundation::IAsyncAction Process_ThreadPage::LoadThreadList()
    {
        if (!processForInfoWindow) co_return;

        auto start = std::chrono::high_resolution_clock::now();

        auto lifetime = get_strong();

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::ThreadInfo> threads;
        threads.reserve(100);

        // 获取线程列表
        try {
            KernelInstance::EnumProcessThread(processForInfoWindow.EProcessULong(), threads);
        }
        catch (const std::exception& e) {
            // 忽略异常
        }

        co_await wil::resume_foreground(DispatcherQueue());

        m_threadList.Clear();
        for (const auto& thread : threads) {
            m_threadList.Append(thread);
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

        // 更新进程数量文本
        std::wstringstream countText;
        countText << L"共 " << m_threadList.Size() << L" 个线程 (" << duration.count() << " ms)";
        ThreadCountText().Text(countText.str());
        threads.clear();
    }


    void Process_ThreadPage::ColumnHeader_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        Button clickedButton = sender.as<Button>();
        winrt::hstring columnName = clickedButton.Tag().as<winrt::hstring>();

        IdHeaderButton().Content(box_value(L"TID"));
        EThreadHeaderButton().Content(box_value(L"ETHREAD"));

        if (columnName == L"Id")
        {
            ApplySort(m_isIdAscending, "Id");
        }
        else {
            LoadThreadList();
        }
    }

    // 排序切换
    void Process_ThreadPage::ApplySort(bool& isAscending, const std::string& column)
    {
        std::vector<winrt::StarlightGUI::ThreadInfo> sortedThreads;

        for (auto& process : m_threadList) {
            sortedThreads.push_back(process);
        }

        if (column == "TID") {
            if (isAscending) {
                IdHeaderButton().Content(box_value(L"TID ↓"));
                std::sort(sortedThreads.begin(), sortedThreads.end(), [](auto a, auto b) {
                    return a.Id() < b.Id();
                    });

            }
            else {
                IdHeaderButton().Content(box_value(L"TID ↑"));
                std::sort(sortedThreads.begin(), sortedThreads.end(), [](auto a, auto b) {
                    return a.Id() > b.Id();
                    });
            }
        }

        m_threadList.Clear();
        for (auto& thread : sortedThreads) {
            m_threadList.Append(thread);
        }

        isAscending = !isAscending;
    }

    template <typename T>
    T Process_ThreadPage::FindParent(DependencyObject const& child)
    {
        DependencyObject parent = VisualTreeHelper::GetParent(child);
        while (parent && !parent.try_as<T>())
        {
            parent = VisualTreeHelper::GetParent(parent);
        }
        return parent.try_as<T>();
    }
}