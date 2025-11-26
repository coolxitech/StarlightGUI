#pragma once

#include "ThreadInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct ThreadInfo : ThreadInfoT<ThreadInfo>
    {
        ThreadInfo() = default;

        int32_t Id();
        void Id(int32_t const& value);

        hstring EThread();
        void EThread(hstring const& value);

        hstring Address();
        void Address(hstring const& value);

        hstring Status();
        void Status(hstring const& value);

        int32_t Priority();
        void Priority(int32_t value);

        hstring ModuleInfo();
        void ModuleInfo(hstring const& value);

    private:
        int32_t m_id{ 0 };
        hstring m_ethread{ L"" };
        hstring m_address{ L"" };
        hstring m_status{ L"" };
        int32_t m_priority{ 0 };
        hstring m_module{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct ThreadInfo : ThreadInfoT<ThreadInfo, implementation::ThreadInfo>
    {
    };
}