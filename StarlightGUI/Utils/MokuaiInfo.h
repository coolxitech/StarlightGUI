#pragma once

#include "MokuaiInfo.g.h"
/*
* 补药问我为什么是Mokuai不是Module，ModuleInfo会报错，我不知道为什么
* @Author Stars
*/
namespace winrt::StarlightGUI::implementation
{
    struct MokuaiInfo : MokuaiInfoT<MokuaiInfo>
    {
        MokuaiInfo() = default;

        hstring Name();
        void Name(hstring const& value);

        hstring Address();
        void Address(hstring const& value);

        hstring Size();
        void Size(hstring const& value);

        hstring Path();
        void Path(hstring const& value);

    private:
        hstring m_name{ L"" };
        hstring m_address{ L"" };
        hstring m_size{ L"" };
        hstring m_path{ L"" };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct MokuaiInfo : MokuaiInfoT<MokuaiInfo, implementation::MokuaiInfo>
    {
    };
}