#pragma once

#include "pch.h"

namespace winrt::StarlightGUI::implementation {
    std::wstring GenerateRandomString(size_t length);

    std::wstring FixBackSplash(const hstring& hstr);

    std::wstring RemoveFromString(const hstring& hstr, const hstring& removeStr);

    std::wstring GetParentDirectory(const hstring& path);

    std::string WideStringToString(const hstring& hstr);

    std::wstring ULongToHexString(ULONG64 value);

    std::wstring ULongToHexString(ULONG64 value, int w, bool uppercase, bool prefix);

    std::wstring FormatMemorySize(uint64_t bytes);

    std::wstring ExtractFunctionName(const std::string& old);
}