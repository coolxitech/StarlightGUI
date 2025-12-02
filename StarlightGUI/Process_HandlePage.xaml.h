#pragma once

#include "Process_HandlePage.g.h"
#include <map>
#include <TlHelp32.h>
#include <winrt/Windows.Foundation.Collections.h>

namespace winrt::StarlightGUI::implementation
{
    struct Process_HandlePage : Process_HandlePageT<Process_HandlePage>
    {
        Process_HandlePage();

        void HandleListView_RightTapped(winrt::Windows::Foundation::IInspectable const& sender, winrt::Microsoft::UI::Xaml::Input::RightTappedRoutedEventArgs const& e);

        winrt::Windows::Foundation::IAsyncAction LoadHandleList();

        winrt::Windows::Foundation::Collections::IObservableVector<winrt::StarlightGUI::HandleInfo> m_handleList{
            winrt::single_threaded_observable_vector<winrt::StarlightGUI::HandleInfo>()
        };

        template <typename T>
        T FindParent(winrt::Microsoft::UI::Xaml::DependencyObject const& child);
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct Process_HandlePage : Process_HandlePageT<Process_HandlePage, implementation::Process_HandlePage>
    {
    };
}
