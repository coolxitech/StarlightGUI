#include "pch.h"
#include "TaskPage.xaml.h"
#if __has_include("TaskPage.g.cpp")
#include "TaskPage.g.cpp"
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
#include <mutex>
#include <shellapi.h>
#include <Utils/TaskUtils.h>
#include <Utils/Utils.h>
#include <Utils/KernelBase.h>
#include <unordered_set>
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
    const static uint64_t KB = 1024;
    const static uint64_t MB = KB * 1024;
    const static uint64_t GB = MB * 1024;
    static std::unordered_map<hstring, std::optional<winrt::Microsoft::UI::Xaml::Media::ImageSource>> iconCache;
    static std::unordered_map<hstring, hstring> descriptionCache;
    static std::unordered_map<DWORD, int> processIndexMap;
    static HDC hdc{ nullptr };
    static std::unordered_set<int> filteredPids;
    static std::vector<winrt::StarlightGUI::ProcessInfo> fullRecordedProcesses;
    static std::mutex safelock;
    static int safeAcceptedPID = -1;
    static std::chrono::steady_clock::time_point lastRefresh;
    static bool infoWindowOpen = false;

    TaskPage::TaskPage() {
        InitializeComponent();

        ProcessListView().ItemsSource(m_processList);
        ProcessListView().ItemContainerTransitions().Clear();
        ProcessListView().ItemContainerTransitions().Append(EntranceThemeTransition());
        TaskUtils::EnsurePrivileges();

        hdc = GetDC(NULL);

        this->Loaded([this](auto&&, auto&&) {
            StartLoop();
			});
    }

    void TaskPage::StartLoop() {
        // 加载一次列表
        LoadProcessList();

        // 每15秒刷新一次列表
        defaultRefreshTimer.Interval(std::chrono::seconds(15));
        defaultRefreshTimer.Tick([this](auto&&, auto&&) {
            if (g_mainWindowInstance->m_openWindows.empty()) LoadProcessList();
            });
        defaultRefreshTimer.Start();

        // 每100秒清除一次缓存
        // 新进程产生相同PID的情况概率很小，所以这个间隔可以大一点
        cacheClearTimer.Interval(std::chrono::seconds(100));
        cacheClearTimer.Tick([this](auto&&, auto&&) {
            iconCache.clear();
            descriptionCache.clear();
            });
        cacheClearTimer.Start();
    }

    void TaskPage::ProcessListView_RightTapped(IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e)
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

        auto item = listView.SelectedItem().as<winrt::StarlightGUI::ProcessInfo>();

        MenuFlyout menuFlyout;

        // 选项1.1
        MenuFlyoutItem item1_1;
        item1_1.Icon(CreateFontIcon(L"\ue711"));
        item1_1.Text(L"结束进程");
        item1_1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::_TerminateProcess(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功结束进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法结束进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 选项1.2
        MenuFlyoutItem item1_2;
        item1_2.Icon(CreateFontIcon(L"\ue8f0"));
        item1_2.Text(L"结束进程 (内核)");
        item1_2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::_ZwTerminateProcess(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功结束进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法结束进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 选项1.3
        MenuFlyoutItem item1_3;
        item1_3.Icon(CreateFontIcon(L"\ue945"));
        item1_3.Text(L"强制结束进程");
        item1_3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (safeAcceptedPID == item.Id()) {
                if (KernelInstance::MurderProcess(item.Id())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功强制结束进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    LoadProcessList();
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法强制结束进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
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
        item2_1.Text(L"设置进程状态");
        MenuFlyoutItem item2_1_sub1;
        item2_1_sub1.Icon(CreateFontIcon(L"\ue769"));
        item2_1_sub1.Text(L"暂停进程");
        item2_1_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::_SuspendProcess(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功暂停进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法暂停进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_1.Items().Append(item2_1_sub1);
        MenuFlyoutItem item2_1_sub2;
        item2_1_sub2.Icon(CreateFontIcon(L"\ue768"));
        item2_1_sub2.Text(L"恢复进程");
        item2_1_sub2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::_ResumeProcess(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功恢复进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法恢复进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_1.Items().Append(item2_1_sub2);

        // 选项2.2
        MenuFlyoutItem item2_2;
        item2_2.Icon(CreateFontIcon(L"\ued1a"));
        item2_2.Text(L"隐藏进程");
        item2_2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::HideProcess(item.Id())) {
                CreateInfoBarAndDisplay(L"成功", L"成功隐藏进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法隐藏进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 选项2.3
        MenuFlyoutSubItem item2_3;
        item2_3.Icon(CreateFontIcon(L"\uea18"));
        item2_3.Text(L"设置PPL等级");
        MenuFlyoutItem item2_3_sub1;

        // PPL等级
        item2_3_sub1.Text(L"None");
        item2_3_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_None)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 None (0x00).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub1);
        MenuFlyoutItem item2_3_sub2;
        item2_3_sub2.Text(L"Authenticode");
        item2_3_sub2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_Authenticode)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 Authenticode (0x11).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub2);
        MenuFlyoutItem item2_3_sub3;
        item2_3_sub3.Text(L"Codegen");
        item2_3_sub3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_Codegen)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 Codegen (0x21).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub3);
        MenuFlyoutItem item2_3_sub4;
        item2_3_sub4.Text(L"Antimalware");
        item2_3_sub4.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_Antimalware)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 Antimalware (0x31).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub4);
        MenuFlyoutItem item2_3_sub5;
        item2_3_sub5.Text(L"Lsa");
        item2_3_sub5.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_Lsa)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 Lsa (0x41).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub5);
        MenuFlyoutItem item2_3_sub6;
        item2_3_sub6.Text(L"Windows");
        item2_3_sub6.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_Windows)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 Windows (0x51).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub6);
        MenuFlyoutItem item2_3_sub7;
        item2_3_sub7.Text(L"WinTcb");
        item2_3_sub7.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_WinTcb)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 WinTcb (0x61).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub7);
        MenuFlyoutItem item2_3_sub8;
        item2_3_sub8.Text(L"WinSystem");
        item2_3_sub8.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (KernelInstance::SetPPL(item.Id(), PPL_WinSystem)) {
                CreateInfoBarAndDisplay(L"成功", L"成功设置进程PPL等级为 WinSystem (0x71).", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法设置进程PPL等级, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item2_3.Items().Append(item2_3_sub8);

        // 选项2.4
        MenuFlyoutItem item2_4;
        item2_4.Icon(CreateFontIcon(L"\ue8c9"));
        item2_4.Text(L"设置为关键进程");
        item2_4.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
                co_return;
            }
            if (safeAcceptedPID == item.Id()) {
                if (KernelInstance::SetCriticalProcess(item.Id())) {
                    CreateInfoBarAndDisplay(L"成功", L"成功设置为关键进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                    LoadProcessList();
                }
                else CreateInfoBarAndDisplay(L"失败", L"无法设置为关键进程: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            }
            else {
                safeAcceptedPID = item.Id();
                CreateInfoBarAndDisplay(L"警告", L"设置为关键进程后，该进程退出会导致蓝屏，如果确认继续请再次点击！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
            }
            co_return;
            });

        MenuFlyoutItem item2_x;
        item2_x.Icon(CreateFontIcon(L"\uf1e8"));
        item2_x.Text(L"效率模式");
        item2_x.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::EnableProcessPerformanceMode(item)) {
                CreateInfoBarAndDisplay(L"成功", L"成功为进程开启效率模式: " + item.Name() + L" (" + to_hstring(item.Id()) + L")", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
                LoadProcessList();
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法为进程开启效率模式: " + item.Name() + L" (" + to_hstring(item.Id()) + L"), 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 分割线2
        MenuFlyoutSeparator separator2;

        MenuFlyoutItem item3_1;
        item3_1.Icon(CreateFontIcon(L"\ue946"));
        item3_1.Text(L"更多信息");
        item3_1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (!KernelInstance::IsRunningAsAdmin()) {
                CreateInfoBarAndDisplay(L"警告", L"使用该功能需要以管理员身份运行该程序！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
            }
            processForInfoWindow = item;
            auto infoWindow = winrt::make<InfoWindow>();
            infoWindow.Activate();
            co_return;
            });

        // 选项3.1
        MenuFlyoutSubItem item3_2;
        item3_2.Icon(CreateFontIcon(L"\ue8c8"));
        item3_2.Text(L"复制信息");
        MenuFlyoutItem item3_1_sub1;
        item3_1_sub1.Icon(CreateFontIcon(L"\ue8ac"));
        item3_1_sub1.Text(L"名称");
        item3_1_sub1.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(item.Name().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item3_2.Items().Append(item3_1_sub1);
        MenuFlyoutItem item3_1_sub2;
        item3_1_sub2.Icon(CreateFontIcon(L"\ue943"));
        item3_1_sub2.Text(L"PID");
        item3_1_sub2.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(std::to_wstring(item.Id()))) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item3_2.Items().Append(item3_1_sub2);
        MenuFlyoutItem item3_1_sub3;
        item3_1_sub3.Icon(CreateFontIcon(L"\uec6c"));
        item3_1_sub3.Text(L"文件路径");
        item3_1_sub3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::CopyToClipboard(item.ExecutablePath().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已复制内容至剪贴板", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法复制内容至剪贴板, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });
        item3_2.Items().Append(item3_1_sub3);

        // 选项5
        MenuFlyoutItem item3_3;
        item3_3.Icon(CreateFontIcon(L"\uec50"));
        item3_3.Text(L"打开文件所在位置");
        item3_3.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::OpenFolderAndSelectFile(item.ExecutablePath().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已打开文件夹", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法打开文件夹, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        // 选项6
        MenuFlyoutItem item3_4;
        item3_4.Icon(CreateFontIcon(L"\ue8ec"));
        item3_4.Text(L"属性");
        item3_4.Click([this, item](IInspectable const& sender, RoutedEventArgs const& e) -> winrt::Windows::Foundation::IAsyncAction {
            if (TaskUtils::OpenFileProperties(item.ExecutablePath().c_str())) {
                CreateInfoBarAndDisplay(L"成功", L"已打开文件属性", InfoBarSeverity::Success, XamlRoot(), InfoBarPanel());
            }
            else CreateInfoBarAndDisplay(L"失败", L"无法打开文件属性, 错误码: " + to_hstring((int)GetLastError()), InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
            co_return;
            });

        menuFlyout.Items().Append(item1_1);
        menuFlyout.Items().Append(item1_2);
        menuFlyout.Items().Append(item1_3);
        menuFlyout.Items().Append(separator1);
        menuFlyout.Items().Append(item2_1);
        menuFlyout.Items().Append(item2_2);
        menuFlyout.Items().Append(item2_3);
        menuFlyout.Items().Append(item2_4);
        menuFlyout.Items().Append(item2_x);
        menuFlyout.Items().Append(separator2);
        menuFlyout.Items().Append(item3_1);
        menuFlyout.Items().Append(item3_2);
        menuFlyout.Items().Append(item3_3);
        menuFlyout.Items().Append(item3_4);

        menuFlyout.ShowAt(listView, e.GetPosition(listView));
    }

    winrt::Windows::Foundation::IAsyncAction TaskPage::LoadProcessList()
    {
        if (m_isLoadingProcesses) {
            co_return;
        }

        m_isLoadingProcesses = true;

        auto start = std::chrono::steady_clock::now();

        auto lifetime = get_strong();
        int selectedItemId = -1;
        if (ProcessListView().SelectedItem()) selectedItemId = ProcessListView().SelectedItem().as<winrt::StarlightGUI::ProcessInfo>().Id();

        winrt::hstring query;
        ProcessSearchBox().Document().GetText(TextGetOptions::NoHidden, query);

        co_await winrt::resume_background();

        std::vector<winrt::StarlightGUI::ProcessInfo> processes;
        std::map<DWORD, hstring> processCpuTable;

        if (std::chrono::duration_cast<std::chrono::seconds>(start - lastRefresh).count() >= 3) {
            processes.reserve(100);

            HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
            if (hSnapshot == INVALID_HANDLE_VALUE) {
                co_await wil::resume_foreground(DispatcherQueue());
                CreateInfoBarAndDisplay(L"错误", L"无法获取进程快照", InfoBarSeverity::Error, XamlRoot(), InfoBarPanel());
                co_return;
            }

            PROCESSENTRY32W pe32{};
            pe32.dwSize = sizeof(PROCESSENTRY32W);

            if (Process32FirstW(hSnapshot, &pe32)) {
                do {
                    co_await GetProcessInfoAsync(pe32, processes);
                } while (Process32NextW(hSnapshot, &pe32));
            }

            CloseHandle(hSnapshot);

            processIndexMap.clear();
            for (int i = 0; i < processes.size(); i++) {
                auto& process = processes.at(i);
                processIndexMap[process.Id()] = i;
            }

            try {
                KernelInstance::EnumProcess(processIndexMap, processes);
            }
            catch (const std::exception& ex) {

            }

            if (query.empty()) fullRecordedProcesses = processes;

            lastRefresh = std::chrono::steady_clock::now();
        }
        else {
            processes = fullRecordedProcesses;
        }

        // 异步加载CPU使用率
        co_await TaskUtils::FetchProcessCpuUsage(processCpuTable);

        co_await wil::resume_foreground(DispatcherQueue());

        m_processList.Clear();
        winrt::StarlightGUI::ProcessInfo& selectedTarget = fullRecordedProcesses[0];
        for (const auto& process : processes) {
            bool shouldRemove = query.empty() ? false : co_await ApplyFilter(process, query);
            if (shouldRemove) continue;

            if (process.CpuUsage().empty()) process.CpuUsage(L"-1 (未知)");
            if (process.MemoryUsage().empty()) process.MemoryUsage(L"-1 (未知)");
            if (process.Status().empty()) process.Status(L"运行中");
            if (process.EProcess().empty()) process.EProcess(L"未知");

			// 从缓存加载图标，没有则获取
            co_await GetProcessIconAsync(process);

            // 加载CPU占用
            if (processCpuTable.find((DWORD)process.Id()) != processCpuTable.end()) process.CpuUsage(processCpuTable[(DWORD)process.Id()]);

            // 寻找选中目标
            if (selectedItemId == process.Id()) selectedTarget = process;

            m_processList.Append(process);
        }

        // 恢复排序
        ApplySort(currentSortingOption, currentSortingType);

        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

		// 更新进程数量文本
        std::wstringstream countText;
        countText << L"共 " << m_processList.Size() << L" 个进程 (" << duration << " ms)";
        ProcessCountText().Text(countText.str());
        processes.clear();

		// 恢复选中项
        uint32_t selectedIndex;
        if (m_processList.IndexOf(selectedTarget, selectedIndex)) {
            ProcessListView().SelectedIndex(selectedIndex);
        }

        m_isLoadingProcesses = false;
    }

    winrt::Windows::Foundation::IAsyncAction TaskPage::GetProcessInfoAsync(const PROCESSENTRY32W& pe32, std::vector<winrt::StarlightGUI::ProcessInfo>& processes)
    {
		// 跳过筛选的PID，在搜索时性能更好
        std::lock_guard<std::mutex> lock(safelock);
        if (filteredPids.find(pe32.th32ProcessID) != filteredPids.end()) {
            co_return;
		}
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
        if (hProcess) {
            WCHAR processName[MAX_PATH] = L"";
            DWORD size = MAX_PATH;

            if (QueryFullProcessImageNameW(hProcess, 0, processName, &size)) {
                uint64_t memoryUsage = TaskUtils::GetProcessWorkingSet(hProcess);
                if (memoryUsage == 0) {
                    PROCESS_MEMORY_COUNTERS pmc{};
                    if (K32GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc))) {
                        memoryUsage = pmc.WorkingSetSize;
                    }
                }

				// 缓存描述信息
                if (descriptionCache.find(processName) == descriptionCache.end()) {
                    DWORD dummy;
                    DWORD versionInfoSize = GetFileVersionInfoSizeW(processName, &dummy);
                    std::wstring description = L"";

                    if (versionInfoSize > 0) {
                        std::vector<BYTE> versionInfo(versionInfoSize);
                        if (GetFileVersionInfoW(processName, 0, versionInfoSize, versionInfo.data())) {
                            VS_FIXEDFILEINFO* fileInfo;
                            UINT fileInfoSize;

                            if (VerQueryValueW(versionInfo.data(), L"\\", (LPVOID*)&fileInfo, &fileInfoSize)) {
                                if (fileInfo->dwFileFlagsMask & VS_FF_INFOINFERRED) {
                                }
                            }

                            wchar_t* fileDescription = nullptr;
                            UINT descriptionSize;
                            if (VerQueryValueW(versionInfo.data(), L"\\StringFileInfo\\040904b0\\FileDescription", (LPVOID*)&fileDescription, &descriptionSize) && fileDescription) {
                                description = fileDescription;
                            }
                        }
                    }

                    if (description.empty()) {
                        description = L"应用程序";
                    }

					descriptionCache[processName] = description;
				}

                auto processInfo = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
                processInfo.Id((int32_t)pe32.th32ProcessID);
                processInfo.Name(pe32.szExeFile);
                processInfo.Description(descriptionCache[processName]);
                processInfo.MemoryUsage(FormatMemorySize(memoryUsage));
                processInfo.MemoryUsageByte(memoryUsage);
                processInfo.ExecutablePath(processName);
                processInfo.Icon(nullptr); // 先设置成null，后面再加载

                processes.push_back(processInfo);
            }

            CloseHandle(hProcess);
        }

        co_return;
    }

    winrt::Windows::Foundation::IAsyncAction TaskPage::GetProcessIconAsync(const winrt::StarlightGUI::ProcessInfo& process) {
        if (iconCache.find(process.ExecutablePath()) == iconCache.end()) {
            SHFILEINFO shfi;
            if (!SHGetFileInfoW(process.ExecutablePath().c_str(), 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON | SHGFI_SMALLICON)) {
                SHGetFileInfoW(L"C:\\Windows\\System32\\ntoskrnl.exe", 0, &shfi, sizeof(SHFILEINFO), SHGFI_ICON);
            }
            auto stream = winrt::Windows::Storage::Streams::InMemoryRandomAccessStream();
            ICONINFO iconInfo;
            if (GetIconInfo(shfi.hIcon, &iconInfo)) {
                BITMAP bmp;
                GetObject(iconInfo.hbmColor, sizeof(bmp), &bmp);
                BITMAPINFOHEADER bmiHeader = { 0 };
                bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
                bmiHeader.biWidth = bmp.bmWidth;
                bmiHeader.biHeight = bmp.bmHeight;
                bmiHeader.biPlanes = 1;
                bmiHeader.biBitCount = 32;
                bmiHeader.biCompression = BI_RGB;

                int dataSize = bmp.bmWidthBytes * bmp.bmHeight;
                std::vector<BYTE> buffer(dataSize);

                GetDIBits(hdc, iconInfo.hbmColor, 0, bmp.bmHeight, buffer.data(), reinterpret_cast<BITMAPINFO*>(&bmiHeader), DIB_RGB_COLORS);

                winrt::Microsoft::UI::Xaml::Media::Imaging::WriteableBitmap writeableBitmap(bmp.bmWidth, bmp.bmHeight);

                // 将数据写入 WriteableBitmap
                uint8_t* data = writeableBitmap.PixelBuffer().data();
                int rowSize = bmp.bmWidth * 4;
                for (int i = 0; i < bmp.bmHeight; ++i) {
                    int srcOffset = i * rowSize;
                    int dstOffset = (bmp.bmHeight - 1 - i) * rowSize;
                    std::memcpy(data + dstOffset, buffer.data() + srcOffset, rowSize);
                }

                DeleteObject(iconInfo.hbmColor);
                DeleteObject(iconInfo.hbmMask);
                DestroyIcon(shfi.hIcon);

                // 将图标缓存到 map 中
                iconCache[process.ExecutablePath()] = writeableBitmap.as<winrt::Microsoft::UI::Xaml::Media::ImageSource>();
            }
        }
        process.Icon(iconCache[process.ExecutablePath()].value());

        co_return;
    }

    winrt::hstring TaskPage::FormatMemorySize(uint64_t bytes)
    {
        std::wstringstream ss;
        ss << std::fixed << std::setprecision(1);

        if (bytes >= GB) {
            ss << (static_cast<double>(bytes) / GB) << " GB";
        }
        else if (bytes >= MB) {
            ss << (static_cast<double>(bytes) / MB) << " MB";
        }
        else if (bytes >= KB) {
            ss << (static_cast<double>(bytes) / KB) << " KB";
        }
        else {
            ss << bytes << " B";
        }

        return to_hstring(ss.str().c_str());
    }

    void TaskPage::ColumnHeader_Click(IInspectable const& sender, RoutedEventArgs const& e)
    {
        Button clickedButton = sender.as<Button>();
        winrt::hstring columnName = clickedButton.Tag().as<winrt::hstring>();

        if (columnName == L"Name")
        {
            ApplySort(m_isNameAscending, "Name");
        }
        else if (columnName == L"CpuUsage")
        {
            ApplySort(m_isCpuAscending, "CpuUsage");
        }
        else if (columnName == L"MemoryUsage")
        {
            ApplySort(m_isMemoryAscending, "MemoryUsage");
        }
        else if (columnName == L"Id")
        {
            ApplySort(m_isIdAscending, "Id");
        }
    }

    // 排序切换
    winrt::fire_and_forget TaskPage::ApplySort(bool& isAscending, const std::string& column)
    {
        NameHeaderButton().Content(box_value(L"进程"));
        CpuHeaderButton().Content(box_value(L"CPU"));
        MemoryHeaderButton().Content(box_value(L"内存"));
        IdHeaderButton().Content(box_value(L"PID"));

        std::vector<winrt::StarlightGUI::ProcessInfo> sortedProcesses;

        for (auto& process : m_processList) {
            sortedProcesses.push_back(process);
        }

        if (column == "Name") {
            if (isAscending) {
                NameHeaderButton().Content(box_value(L"进程 ↓"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName < bName;
                    });
                
            }
            else {
                NameHeaderButton().Content(box_value(L"进程 ↑"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    std::wstring aName = a.Name().c_str();
                    std::wstring bName = b.Name().c_str();
                    std::transform(aName.begin(), aName.end(), aName.begin(), ::towlower);
                    std::transform(bName.begin(), bName.end(), bName.begin(), ::towlower);

                    return aName > bName;
                    });
            }
        }
        else if (column == "CpuUsage") {
            if (isAscending) {
                CpuHeaderButton().Content(box_value(L"CPU ↓"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    return std::stod(a.CpuUsage().c_str()) < std::stod(b.CpuUsage().c_str());
                    });
            }
            else {
                CpuHeaderButton().Content(box_value(L"CPU ↑"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    return std::stod(a.CpuUsage().c_str()) > std::stod(b.CpuUsage().c_str());
                    });
            }
        }
        else if (column == "MemoryUsage") {
            if (isAscending) {
                MemoryHeaderButton().Content(box_value(L"内存 ↓"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    return a.MemoryUsageByte() < b.MemoryUsageByte();
                    });
            }
            else {
                MemoryHeaderButton().Content(box_value(L"内存 ↑"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    return a.MemoryUsageByte() > b.MemoryUsageByte();
                    });
            }
        }
        else if (column == "Id") {
            if (isAscending) {
                IdHeaderButton().Content(box_value(L"PID ↓"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    return a.Id() < b.Id();
                    });
            }
            else {
                IdHeaderButton().Content(box_value(L"PID ↑"));
                std::sort(sortedProcesses.begin(), sortedProcesses.end(), [](auto a, auto b) {
                    return a.Id() > b.Id();
                    });
            }
        }

        m_processList.Clear();
        for (auto& process : sortedProcesses) {
            m_processList.Append(process);
        }

        isAscending = !isAscending;
        currentSortingOption = !isAscending;
        currentSortingType = column;

        co_return;
    }

    winrt::fire_and_forget TaskPage::ProcessSearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e)
    {
		// 每次搜索都清空之前缓存的过滤结果
        filteredPids.clear();

        winrt::hstring query;
        ProcessSearchBox().Document().GetText(TextGetOptions::NoHidden, query);

        /*
        * 我们的思路是这样的：
        *  - 在LoadProcessList()中，如果搜索框为空，则缓存一次完整的进程列表
        *  - 搜索时，先遍历这个完整的进程列表，记录需要过滤掉的进程PID
        *  - 由于在遍历进程时，我们会先检查一遍filteredPids，所以这会直接跳过进程的所有处理
        *  - 原先的逻辑是每次搜索都重新获取进程列表，然后再筛选一遍得到需要添加到ListView的进程，这意味着我们仍然会去处理进程，即使它最终会被过滤掉
        *  - 这样可以大幅提升搜索性能，并且我们会在添加进程时再进程一次过滤，确保新增的进程也会被正常过滤
        *  - 唯一的缺点是，如果正好原先有个进程退出，然后新启动了一个进程，这个新进程的PID正好是之前被过滤掉的进程的PID，那么这个新进程就会被错误地过滤掉
        *  - 但这种情况发生的概率极低，而且影响也不大，所以可以接受，我们还有个刷新按钮可以手动刷新进程列表
        */
        if (!query.empty()) {
            std::lock_guard<std::mutex> lock(safelock);
            for (const auto& process : fullRecordedProcesses) {
                ApplyFilter(process, query);
            }
        }

        co_await LoadProcessList();
    }

    winrt::Windows::Foundation::IAsyncOperation<bool> TaskPage::ApplyFilter(const winrt::StarlightGUI::ProcessInfo& process, hstring& query) {
        std::wstring name = process.Name().c_str();
        std::wstring queryWStr = query.c_str();

        // 不比较大小写
        std::transform(name.begin(), name.end(), name.begin(), ::towlower);
        std::transform(queryWStr.begin(), queryWStr.end(), queryWStr.begin(), ::towlower);

        uint32_t index;
        bool result = name.find(queryWStr) == std::wstring::npos;
        if (result) {
			// 缓存过滤的PID，这样下次可以直接跳过
			filteredPids.insert(process.Id());
        }

        co_return result;
    }


    winrt::fire_and_forget TaskPage::RefreshProcessListButton_Click(IInspectable const&, RoutedEventArgs const&)
    {
        // 手动刷新时清空过滤列表，确保获取最新的进程列表
        auto current = std::chrono::steady_clock::now();

        if (std::chrono::duration_cast<std::chrono::seconds>(current - lastRefresh).count() < 1) {
            CreateInfoBarAndDisplay(L"警告", L"刷新速度过快，请稍后再试！", InfoBarSeverity::Warning, XamlRoot(), InfoBarPanel());
            co_return;
        }

        RefreshProcessListButton().IsEnabled(false);

        filteredPids.clear();

        co_await LoadProcessList();

        // 重启计时器
        defaultRefreshTimer.Stop();
        defaultRefreshTimer.Start();

        RefreshProcessListButton().IsEnabled(true);
        co_return;
    }

    void TaskPage::OnNavigatedFrom(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        defaultRefreshTimer.Stop();
        cacheClearTimer.Stop();
        ReleaseDC(NULL, hdc);
    }

    TaskPage::~TaskPage() {
        defaultRefreshTimer.Stop();
        cacheClearTimer.Stop();
        ReleaseDC(NULL, hdc);
    }

    template <typename T>
    T TaskPage::FindParent(DependencyObject const& child)
    {
        DependencyObject parent = VisualTreeHelper::GetParent(child);
        while (parent && !parent.try_as<T>())
        {
            parent = VisualTreeHelper::GetParent(parent);
        }
        return parent.try_as<T>();
    }
}