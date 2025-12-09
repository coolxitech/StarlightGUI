#include "pch.h"
#include "FileInfo.h"
#if __has_include("FileInfo.g.cpp")
#include "FileInfo.g.cpp"
#endif

using namespace winrt;
using namespace Microsoft::UI::Xaml::Media;

namespace winrt::StarlightGUI::implementation
{

    hstring FileInfo::Name()
    {
        return m_name;
    }

    void FileInfo::Name(hstring const& value)
    {
        m_name = value;
    }

    hstring FileInfo::Path()
    {
        return m_path;
    }

    void FileInfo::Path(hstring const& value)
    {
        m_path = value;
    }

    bool FileInfo::Directory() 
    {
        return m_directory;
    }

    void FileInfo::Directory(bool const& value)
    {
        m_directory = value;
    }

    ULONG FileInfo::Flag()
    {
        return m_flag;
    }

    void FileInfo::Flag(ULONG const& value)
    {
        m_flag = value;
    }

    ULONG64 FileInfo::Size()
    {
        return m_size;
    }

    void FileInfo::Size(ULONG64 const& value)
    {
        m_size = value;
    }

    ULONG64 FileInfo::MFTID()
    {
        return m_mftId;
    }

    void FileInfo::MFTID(ULONG64 const& value)
    {
        m_mftId = value;
    }

    ImageSource FileInfo::Icon()
    {
        return m_icon;
    }

    void FileInfo::Icon(ImageSource const& value)
    {
        m_icon = value;
    }
}