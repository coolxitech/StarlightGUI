#pragma once

#include "HandleInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct HandleInfo : HandleInfoT<HandleInfo>
    {
        HandleInfo() = default;

        hstring Type();
        void Type(hstring const& value);

        hstring Object();
        void Object(hstring const& value);

        hstring Handle();
        void Handle(hstring const& value);

        hstring Access();
        void Access(hstring const& value);

        hstring Attributes();
        void Attributes(hstring const& value);

    private:
        hstring m_type{ L"" };
        hstring m_object{ L"" };
        hstring m_handle{ L"" };
        hstring m_access{ L"" };
        hstring m_attributes{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct HandleInfo : HandleInfoT<HandleInfo, implementation::HandleInfo>
    {
    };
}