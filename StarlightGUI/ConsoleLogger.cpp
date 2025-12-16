#include "pch.h"
#include "ConsoleLogger.h"
#include <fstream>
#include <codecvt>
#include <locale>
#include <iostream>
#include <filesystem>


static ConsoleLogger* g_instance = nullptr;
static std::mutex g_instanceMutex;

ConsoleLogger& ConsoleLogger::GetInstance() {
    std::lock_guard<std::mutex> lock(g_instanceMutex);
    if (!g_instance) {
        g_instance = new ConsoleLogger();
    }
    return *g_instance;
}

void ConsoleLogger::Destroy() {
    std::lock_guard<std::mutex> lock(g_instanceMutex);
    if (g_instance) {
        delete g_instance;
        g_instance = nullptr;
    }
}

ConsoleLogger::ConsoleLogger() {
    m_colorMap[LogLevel::DEBUG] = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY;     // 青色
    m_colorMap[LogLevel::INFO] = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;            // 白色
    m_colorMap[LogLevel::WARNING] = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;    // 黄色
    m_colorMap[LogLevel::ERROR] = FOREGROUND_RED | FOREGROUND_INTENSITY;                         // 亮红色
    m_colorMap[LogLevel::CRITICAL] = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;    // 品红色
    m_colorMap[LogLevel::SUCCESS] = FOREGROUND_GREEN | FOREGROUND_INTENSITY;                     // 亮绿色
}

ConsoleLogger::~ConsoleLogger() {
    m_shutdown = true;
    m_queueCV.notify_all();
    m_fileQueueCV.notify_all();

    // 等待控制台线程结束
    if (m_consoleThread.joinable()) {
        m_consoleThread.join();
    }

    // 等待文件写入线程结束
    if (m_fileWriteThread.joinable()) {
        m_fileWriteThread.join();
    }

    // 关闭文件
    {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        if (m_logFile.is_open()) {
            m_logFile.close();
        }
    }

    if (m_consoleOpen) {
        ShutdownConsole();
    }
}

BOOL WINAPI HandleConsoleControl(DWORD type) {
    if (type == CTRL_CLOSE_EVENT || type == CTRL_LOGOFF_EVENT || type == CTRL_SHUTDOWN_EVENT || type == CTRL_C_EVENT) {
        ConsoleLogger::GetInstance().CloseConsole();
        return true;
    }
    return false;
}

bool ConsoleLogger::InitializeLogFile() {
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath) == 0) {
        return false;
    }

    m_logFilePath = std::wstring(tempPath) + L"StarlightGUI.log";

    {
        std::lock_guard<std::mutex> lock(m_fileMutex);
        m_logFile.open(m_logFilePath, std::ios::out | std::ios::trunc);
        if (!m_logFile.is_open()) {
            return false;
        }

        try {
            m_logFile.imbue(std::locale(std::locale(), new std::codecvt_utf8<wchar_t>()));
        }
        catch (...) {
            
        }

        m_logFile << L"========= Starlight GUI Log =========\n";
        m_logFile << L"Created: " << FormatTimestamp(std::chrono::system_clock::now()) << L"\n";
        m_logFile << L"Version: " << winrt::unbox_value<hstring>(Application::Current().Resources().TryLookup(box_value(L"Version"))) << L"\n";
        m_logFile << L"=====================================\n\n";
    }
    
    return true;
}

bool ConsoleLogger::Initialize() {
    if (m_initialized) {
        return true;
    }

    if (m_fileWriteEnabled) {
        if (!InitializeLogFile()) {
            m_fileWriteEnabled = false;
        }
    }

    if (!AllocConsole()) {
        if (GetLastError() != ERROR_ACCESS_DENIED) {
            return false;
        }
    }

    m_hConsoleOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    m_hConsoleInput = GetStdHandle(STD_INPUT_HANDLE);

    if (m_hConsoleOutput == INVALID_HANDLE_VALUE || m_hConsoleOutput == nullptr) {
        return false;
    }

    m_hConsoleWnd = GetConsoleWindow();
    ShowWindow(m_hConsoleWnd, SW_HIDE);

    SetConsoleTitleW(m_consoleTitle.c_str());
    SetConsoleCtrlHandler(HandleConsoleControl, TRUE);

    DWORD dwMode = 0;
    if (GetConsoleMode(m_hConsoleOutput, &dwMode)) {
        dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING | ENABLE_PROCESSED_OUTPUT;
        SetConsoleMode(m_hConsoleOutput, dwMode);
    }

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    HMENU hMenu = GetSystemMenu(m_hConsoleWnd, FALSE);
    EnableMenuItem(hMenu, SC_CLOSE, MF_GRAYED);

    // 启动控制台线程
    m_consoleThread = std::thread(&ConsoleLogger::ConsoleThreadProc, this);

    // 启动文件写入线程
    if (m_fileWriteEnabled) {
        m_fileWriteThread = std::thread(&ConsoleLogger::FileWriteThreadProc, this);
    }

    LOG_WARNING(L"", L"Logger initialized. Close the logger window will lead to application termination.");

    if (m_fileWriteEnabled) {
        LOG_INFO(L"", L"Log file: %s", m_logFilePath.c_str());
    }

    m_initialized = true;
    return true;
}

void ConsoleLogger::FileWriteThreadProc() {
    while (!m_shutdown) {
        ProcessFileQueue();

        // 使用条件变量等待新日志，而不是频繁轮询
        std::unique_lock<std::mutex> lock(m_fileQueueMutex);
        if (m_fileLogQueue.empty()) {
            m_fileQueueCV.wait_for(lock, std::chrono::milliseconds(100), [this]() {
                return !m_fileLogQueue.empty() || m_shutdown;
                });
        }
        lock.unlock();
    }

    // 线程结束前处理剩余日志
    ProcessFileQueue();
}

void ConsoleLogger::ProcessFileQueue() {
    std::queue<LogEntry> queueCopy;

    {
        std::lock_guard<std::mutex> lock(m_fileQueueMutex);
        if (m_fileLogQueue.empty()) {
            return;
        }
        queueCopy = std::move(m_fileLogQueue);
    }

    while (!queueCopy.empty()) {
        auto& entry = queueCopy.front();

        // 写入文件
        if (m_fileWriteEnabled) {
            std::wstring formatted;
            try {
                formatted = FormatLogEntry(entry);
            }
            catch (...) {
                // 格式化失败，忽略继续处理下一条
                queueCopy.pop();
                continue;
            }
            std::lock_guard<std::mutex> lock(m_fileMutex);
            if (m_logFile.is_open()) {
                try {
                    m_logFile << formatted << L"\n";
                    m_logFile.flush();  // 立即刷新缓冲区，确保日志及时写入
                }
                catch (...) {
                    // 写入失败，忽略继续处理下一条
                }
            }
        }

        queueCopy.pop();
    }
}

std::wstring ConsoleLogger::FormatLogEntry(const LogEntry& entry) {
    std::wstringstream ss;

    if (m_showTimestamp) {
        ss << L"[" << FormatTimestamp(entry.timestamp) << L"] ";
    }

    if (m_showLogLevel) {
        ss << L"[" << FormatLevel(entry.level) << L"] ";
    }

    if (m_showSource && !entry.source.empty()) {
        ss << L"[" << entry.source << L"] ";
    }

    ss << entry.message;

    return ss.str();
}

bool ConsoleLogger::OpenConsole() {
    if (m_consoleOpen) {
        return true;
    }

    ShowWindow(m_hConsoleWnd, SW_SHOW);
    m_consoleOpen = true;

    return true;
}

bool ConsoleLogger::CloseConsole() {
    if (!m_consoleOpen) {
        return true;
    }

    ShowWindow(m_hConsoleWnd, SW_HIDE);
    m_consoleOpen = false;

    return true;
}

void ConsoleLogger::ToggleConsole() {
    if (m_consoleOpen) CloseConsole();
    else OpenConsole();
}

void ConsoleLogger::ShutdownConsole() {
    if (!m_consoleOpen) {
        return;
    }

    m_consoleOpen = false;

    std::unique_lock<std::mutex> lock(m_queueMutex);
    m_queueCV.wait_for(lock, std::chrono::seconds(0), [this]() {
        return m_logQueue.empty();
        });

    if (m_hConsoleWnd) {
        FreeConsole();
    }

    m_hConsoleOutput = INVALID_HANDLE_VALUE;
    m_hConsoleInput = INVALID_HANDLE_VALUE;
    m_hConsoleWnd = nullptr;
}

void ConsoleLogger::SetTitle(const std::wstring& title) {
    m_consoleTitle = title;
    if (m_consoleOpen && m_hConsoleWnd) {
        SetConsoleTitleW(title.c_str());
    }
}

void ConsoleLogger::ConsoleThreadProc() {
    while (!m_shutdown) {
        ProcessLogQueue();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    ProcessLogQueue();
}

void ConsoleLogger::ProcessLogQueue() {
    std::queue<LogEntry> queueCopy;

    {
        std::lock_guard<std::mutex> lock(m_queueMutex);
        queueCopy = std::move(m_logQueue);
    }

    while (!queueCopy.empty()) {
        auto& entry = queueCopy.front();
        {
            std::lock_guard<std::mutex> lock(m_historyMutex);
            m_logHistory.push_back(entry);

            if (m_logHistory.size() > m_maxHistorySize) {
                m_logHistory.erase(m_logHistory.begin());
            }
        }

        if (m_initialized && entry.level >= m_minLogLevel) {
            OutputToConsole(entry);
        }

        queueCopy.pop();
    }
}

void ConsoleLogger::OutputToConsole(const LogEntry& entry) {
    if (!m_hConsoleOutput || m_hConsoleOutput == INVALID_HANDLE_VALUE) {
        return;
    }

    std::wstring output = FormatLogEntry(entry) + L"\n";

    SetConsoleColor(GetLevelColor(entry.level));

    DWORD charsWritten;
    WriteConsoleW(m_hConsoleOutput, output.c_str(), static_cast<DWORD>(output.length()), &charsWritten, nullptr);

    ResetConsoleColor();
}

WORD ConsoleLogger::GetLevelColor(LogLevel level) {
    auto it = m_colorMap.find(level);
    if (it != m_colorMap.end()) {
        return it->second;
    }
    return FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
}

void ConsoleLogger::SetConsoleColor(WORD color) {
    if (m_hConsoleOutput && m_hConsoleOutput != INVALID_HANDLE_VALUE) {
        SetConsoleTextAttribute(m_hConsoleOutput, color);
    }
}

void ConsoleLogger::ResetConsoleColor() {
    SetConsoleColor(FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

std::wstring ConsoleLogger::FormatTimestamp(const std::chrono::system_clock::time_point& time) {
    auto t = std::chrono::system_clock::to_time_t(time);

    std::wstringstream ss;
    std::tm tm;
    localtime_s(&tm, &t);
    ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S");

    return ss.str();
}

std::wstring ConsoleLogger::FormatLevel(LogLevel level) {
    switch (level) {
    case LogLevel::DEBUG:    return L"DEBUG";
    case LogLevel::INFO:     return L"INFO";
    case LogLevel::WARNING:  return L"WARN";
    case LogLevel::ERROR:    return L"ERROR";
    case LogLevel::CRITICAL: return L"CRITICAL";
    case LogLevel::SUCCESS:  return L"SUCCESS";
    default:                 return L"UNKNOWN";
    }
}

void ConsoleLogger::SetMinLogLevel(LogLevel minLevel) {
    m_minLogLevel = minLevel;
}

void ConsoleLogger::ClearConsole() {
    if (!m_consoleOpen || !m_hConsoleOutput) {
        return;
    }

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD count;
    DWORD cellCount;
    COORD homeCoords = { 0, 0 };

    if (!GetConsoleScreenBufferInfo(m_hConsoleOutput, &csbi)) {
        return;
    }

    cellCount = csbi.dwSize.X * csbi.dwSize.Y;

    if (!FillConsoleOutputCharacterW(
        m_hConsoleOutput,
        L' ',
        cellCount,
        homeCoords,
        &count
    )) {
        return;
    }

    if (!FillConsoleOutputAttribute(
        m_hConsoleOutput,
        csbi.wAttributes,
        cellCount,
        homeCoords,
        &count
    )) {
        return;
    }

    SetConsoleCursorPosition(m_hConsoleOutput, homeCoords);
}

bool ConsoleLogger::SaveToFile(const std::wstring& filePath) {
    std::wofstream file(filePath, std::ios::out | std::ios::trunc);
    if (!file.is_open()) {
        return false;
    }

    std::lock_guard<std::mutex> lock(m_historyMutex);

    file << L"\xEF\xBB\xBF";

    for (const auto& entry : m_logHistory) {
        if (m_showTimestamp) {
            file << L"[" << FormatTimestamp(entry.timestamp) << L"] ";
        }

        if (m_showLogLevel) {
            file << L"[" << FormatLevel(entry.level) << L"] ";
        }

        if (m_showSource && !entry.source.empty()) {
            file << L"[" << entry.source << L"] ";
        }

        file << entry.message << "\n";
    }

    file.close();
    return true;
}

void ConsoleLogger::SetConsolePosition(int x, int y, int width, int height) {
    if (!m_hConsoleWnd) {
        return;
    }

    MoveWindow(m_hConsoleWnd, x, y, width, height, TRUE);
}

std::vector<LogEntry> ConsoleLogger::GetLogHistory() {
    std::lock_guard<std::mutex> lock(m_historyMutex);
    return m_logHistory;
}

HWND ConsoleLogger::GetConsoleHandle() {
    return m_hConsoleWnd;
}

void ConsoleLogger::SetShowTimestamp(bool show) {
    m_showTimestamp = show;
}

void ConsoleLogger::SetShowLogLevel(bool show) {
    m_showLogLevel = show;
}

void ConsoleLogger::SetShowSource(bool show) {
    m_showSource = show;
}