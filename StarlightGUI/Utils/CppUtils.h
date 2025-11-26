#pragma once

#include "pch.h"

namespace winrt::StarlightGUI::implementation {
    std::wstring GenerateRandomString(size_t length);
    std::wstring ULongToHexString(ULONG64 value);
}