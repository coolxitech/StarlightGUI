#pragma once

#include "FileInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct FileInfo : FileInfoT<FileInfo>
    {
        FileInfo() = default;

        hstring Name();
        void Name(hstring const& value);

        hstring Path();
        void Path(hstring const& value);

        bool Directory();
        void Directory(bool const& value);

        ULONG Flag();
        void Flag(ULONG const& value);

        ULONG64 Size();
        void Size(ULONG64 const& value);

        ULONG64 MFTID();
        void MFTID(ULONG64 const& value);

        winrt::Microsoft::UI::Xaml::Media::ImageSource Icon();
        void Icon(winrt::Microsoft::UI::Xaml::Media::ImageSource const& value);

    private:
        hstring m_name{ L"" };
        hstring m_path{ L"" };
        ULONG m_flag{ 0 };
        ULONG64 m_size{ 0 };
        ULONG64 m_mftId{ 0 };
        bool m_directory{ false };
        winrt::Microsoft::UI::Xaml::Media::ImageSource m_icon{ nullptr };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct FileInfo : FileInfoT<FileInfo, implementation::FileInfo>
    {
    };
}