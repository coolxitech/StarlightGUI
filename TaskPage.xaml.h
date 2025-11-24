#pragma once

#include "TaskPage.g.h"
#include <map>
#include <TlHelp32.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::StarlightGUI::implementation
{
    struct TaskPage : TaskPageT<TaskPage>
    {
        TaskPage();

        ~TaskPage();

        void StartLoop();

        winrt::fire_and_forget RefreshProcessListButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void ProcessListView_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e);
        void OnNavigatedFrom(winrt::Microsoft::UI::Xaml::Navigation::NavigationEventArgs const& e);

        void ColumnHeader_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget ApplySort(bool& isAscending, const std::string& column);

        winrt::fire_and_forget ProcessSearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::Windows::Foundation::IAsyncOperation<bool> ApplyFilter(const winrt::StarlightGUI::ProcessInfo& process, hstring& query);

        // 进程列表相关方法
        winrt::Windows::Foundation::IAsyncAction LoadProcessList();
        winrt::Windows::Foundation::IAsyncAction GetProcessInfoAsync(const PROCESSENTRY32W& pe32, std::vector<winrt::StarlightGUI::ProcessInfo>& processes);
        winrt::Windows::Foundation::IAsyncAction GetProcessIconAsync(const winrt::StarlightGUI::ProcessInfo& process);

        winrt::hstring FormatMemorySize(uint64_t bytes);

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::StarlightGUI::ProcessInfo> m_processList{
            winrt::multi_threaded_observable_vector<winrt::StarlightGUI::ProcessInfo>()
        };

        bool m_isLoadingProcesses = false;
        winrt::Microsoft::UI::Xaml::DispatcherTimer defaultRefreshTimer;
        winrt::Microsoft::UI::Xaml::DispatcherTimer cacheClearTimer;

        inline static bool m_isLoading = false;
        inline static bool m_isNameAscending = true;
        inline static bool m_isCpuAscending = true;
        inline static bool m_isMemoryAscending = true;
        inline static bool m_isIdAscending = true;
        inline static bool currentSortingOption;
        inline static std::string currentSortingType;

        template <typename T>
        T FindParent(winrt::Microsoft::UI::Xaml::DependencyObject const& child);
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct TaskPage : TaskPageT<TaskPage, implementation::TaskPage>
    {
    };
}
