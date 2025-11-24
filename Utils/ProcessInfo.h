#pragma once

#include "ProcessInfo.g.h"

namespace winrt::StarlightGUI::implementation
{
    struct ProcessInfo : ProcessInfoT<ProcessInfo>
    {
        ProcessInfo() = default;

        int32_t Id();
        void Id(int32_t const& value);

        hstring Name();
        void Name(hstring const& value);

        hstring Description();
        void Description(hstring const& value);

        hstring MemoryUsage();
        void MemoryUsage(hstring const& value);

        uint64_t MemoryUsageByte();
        void MemoryUsageByte(uint64_t const& value);

        hstring CpuUsage();
        void CpuUsage(hstring const& value);

        hstring ExecutablePath();
        void ExecutablePath(hstring const& value);

        hstring EProcess();
        void EProcess(hstring const& value);

        ULONG64 EProcessULong();
        void EProcessULong(ULONG64 const& value);

        hstring Status();
        void Status(hstring const& value);

        winrt::Microsoft::UI::Xaml::Media::ImageSource Icon();
        void Icon(winrt::Microsoft::UI::Xaml::Media::ImageSource const& value);

    private:
        int32_t m_id{ 0 };
        hstring m_name{ L"" };
        hstring m_description{ L"" };
        hstring m_memoryUsage{ L"" };
        uint64_t m_memoryUsageByte{ 0 };
        hstring m_cpuUsage{ L"" };
        hstring m_executablePath{ L"" };
        hstring m_eprocess{ L"" };
        ULONG64 m_eprocessULong{ 0 };
        hstring m_status{ L"" };
        winrt::Microsoft::UI::Xaml::Media::ImageSource m_icon{ nullptr };
    };
}

namespace winrt::StarlightGUI::factory_implementation
{
    struct ProcessInfo : ProcessInfoT<ProcessInfo, implementation::ProcessInfo>
    {
    };
}