#include "pch.h"
#include "HandleInfo.h"
#if __has_include("HandleInfo.g.cpp")
#include "HandleInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    void HandleInfo::Type(hstring const& value)
    {
        m_type = value;
    }

    hstring HandleInfo::Type()
    {
        return m_type;
    }

    void HandleInfo::Object(hstring const& value)
    {
        m_object = value;
    }

    hstring HandleInfo::Object()
    {
        return m_object;
    }

    void HandleInfo::Handle(hstring const& value)
    {
        m_handle = value;
    }

    hstring HandleInfo::Handle()
    {
        return m_handle;
    }

    void HandleInfo::Access(hstring const& value)
    {
        m_access = value;
    }

    hstring HandleInfo::Access()
    {
        return m_access;
    }

    hstring HandleInfo::Attributes()
    {
        return m_attributes;
    }

    void HandleInfo::Attributes(hstring const& value)
    {
        m_attributes = value;
    }
}