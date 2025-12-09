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

    hstring FileInfo::ModifyTime()
    {
        return m_modifyTime;
    }

    void FileInfo::ModifyTime(hstring const& value)
    {
        m_modifyTime = value;
    }

    ULONG64 FileInfo::ModifyTimeULong()
    {
        return m_modifyTimeULong;
    }

    void FileInfo::ModifyTimeULong(ULONG64 const& value)
    {
        m_modifyTimeULong = value;
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

    hstring FileInfo::Size()
    {
        return m_size;
    }

    void FileInfo::Size(hstring const& value)
    {
        m_size = value;
    }

    ULONG64 FileInfo::SizeULong()
    {
        return m_sizeULong;
    }

    void FileInfo::SizeULong(ULONG64 const& value)
    {
        m_sizeULong = value;
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