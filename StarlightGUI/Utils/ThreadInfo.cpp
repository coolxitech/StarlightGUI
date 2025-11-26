#include "pch.h"
#include "ThreadInfo.h"
#if __has_include("ThreadInfo.g.cpp")
#include "ThreadInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    int32_t ThreadInfo::Id()
    {
        return m_id;
    }

    void ThreadInfo::Id(int32_t const& value)
    {
        m_id = value;
    }

    hstring ThreadInfo::EThread()
    {
        return m_ethread;
    }

    void ThreadInfo::EThread(hstring const& value)
    {
        m_ethread = value;
    }

    hstring ThreadInfo::Address()
    {
        return m_address;
    }

    void ThreadInfo::Address(hstring const& value)
    {
        m_address = value;
    }

    hstring ThreadInfo::Status()
    {
        return m_status;
    }

    void ThreadInfo::Status(hstring const& value)
    {
        m_status = value;
    }

    int32_t ThreadInfo::Priority()
    {
        return m_priority;
    }

    void ThreadInfo::Priority(int32_t value)
    {
        m_priority = value;
    }

    hstring ThreadInfo::ModuleInfo()
    {
        return m_module;
    }

    void ThreadInfo::ModuleInfo(hstring const& value)
    {
        m_module = value;
    }
}