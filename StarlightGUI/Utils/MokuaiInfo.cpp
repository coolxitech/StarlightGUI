#include "pch.h"
#include "MokuaiInfo.h"
#if __has_include("MokuaiInfo.g.cpp")
#include "MokuaiInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;
/*
* 补药问我为什么是Mokuai不是Module，ModuleInfo会报错，我不知道为什么
* @Author Stars
*/
namespace winrt::StarlightGUI::implementation
{


    hstring MokuaiInfo::Name()
    {
        return m_name;
    }

    void MokuaiInfo::Name(hstring const& value)
    {
        m_name = value;
    }

    hstring MokuaiInfo::Address()
    {
        return m_address;
    }

    void MokuaiInfo::Address(hstring const& value)
    {
        m_address = value;
    }

    hstring MokuaiInfo::Size()
    {
        return m_size;
    }

    void MokuaiInfo::Size(hstring const& value)
    {
        m_size = value;
    }

    hstring MokuaiInfo::Path()
    {
        return m_path;
    }

    void MokuaiInfo::Path(hstring const& value)
    {
        m_path = value;
    }
}