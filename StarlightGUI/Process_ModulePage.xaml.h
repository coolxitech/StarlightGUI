#pragma once

#include "Process_ModulePage.g.h"
#include <map>
#include <TlHelp32.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::StarlightGUI::implementation
{
    struct Process_ModulePage : Process_ModulePageT<Process_ModulePage>
    {
        Process_ModulePage();

        void ModuleListView_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e);

        winrt::Windows::Foundation::IAsyncAction LoadModuleList();

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::StarlightGUI::MokuaiInfo> m_moduleList{
            winrt::single_threaded_observable_vector<winrt::StarlightGUI::MokuaiInfo>()
        };

        template <typename T>
        T FindParent(winrt::Microsoft::UI::Xaml::DependencyObject const& child);
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct Process_ModulePage : Process_ModulePageT<Process_ModulePage, implementation::Process_ModulePage>
    {
    };
}
