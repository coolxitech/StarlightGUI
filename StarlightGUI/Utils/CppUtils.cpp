#include "pch.h"
#include "CppUtils.h"

#include <sstream>
#include <iomanip>
#include <iostream>
#include <string>
#include <random>
#include <limits>

namespace winrt::StarlightGUI::implementation {
    std::wstring GenerateRandomString(size_t length) {
        const std::wstring charset =
            L"0123456789"
            L"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
            L"abcdefghijklmnopqrstuvwxyz";

        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dist(0, static_cast<int>(charset.size() - 1));

        std::wstring result;
        result.reserve(length);

        for (size_t i = 0; i < length; ++i) {
            result += charset[dist(gen)];
        }
        return result;
    }

    std::wstring ULongToHexString(ULONG64 value)
    {
        return ULongToHexString(value, 16, true, true);
    }

    std::wstring ULongToHexString(ULONG64 value, int w, bool uppercase, bool prefix) {
        std::wstringstream ss;
        if (prefix) ss << L"0x";
        if (w > 0) ss << std::setw(w);
        ss << std::setfill(L'0') << std::hex;
        if (uppercase) ss << std::uppercase;
        ss << value;
        return ss.str();
    }
}