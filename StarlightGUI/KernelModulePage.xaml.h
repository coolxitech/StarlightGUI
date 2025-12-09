#pragma once

#include "KernelModulePage.g.h"
#include <map>
#include <TlHelp32.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::StarlightGUI::implementation
{
    struct KernelModulePage : KernelModulePageT<KernelModulePage>
    {
        KernelModulePage();

        winrt::fire_and_forget RefreshKernelModuleListButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget UnloadModuleButton_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget LoadDriverButton_Click(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        void KernelModuleListView_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e);

        void ColumnHeader_Click(IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        winrt::fire_and_forget ApplySort(bool& isAscending, const std::string& column);

        void KernelModuleSearchBox_TextChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::RoutedEventArgs const& e);
        bool ApplyFilter(const winrt::StarlightGUI::KernelModuleInfo& kernelModule, hstring& query);

        winrt::Windows::Foundation::IAsyncAction LoadKernelModuleList();
        winrt::Windows::Foundation::IAsyncAction WaitAndReloadAsync(int interval);

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::StarlightGUI::KernelModuleInfo> m_kernelModuleList{
            winrt::multi_threaded_observable_vector<winrt::StarlightGUI::KernelModuleInfo>()
        };

        bool m_isLoadingKernelModules = false;

        winrt::Microsoft::UI::Xaml::DispatcherTimer reloadTimer;

        inline static bool m_isLoading = false;
        inline static bool m_isNameAscending = true;
        inline static bool m_isSizeAscending = true;
        inline static bool m_isLoadOrderAscending = true;
        inline static bool currentSortingOption;
        inline static std::string currentSortingType;

        template <typename T>
        T FindParent(winrt::Microsoft::UI::Xaml::DependencyObject const& child);
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct KernelModulePage : KernelModulePageT<KernelModulePage, implementation::KernelModulePage>
    {
    };
}
