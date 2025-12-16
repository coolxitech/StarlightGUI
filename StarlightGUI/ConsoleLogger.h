#pragma once
#include <windows.h>
#include <string>
#include <vector>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <atomic>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <map>
#include <functional>
#include <fstream>
#undef ERROR

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    CRITICAL,
    SUCCESS
};

struct LogEntry {
    std::chrono::system_clock::time_point timestamp;
    LogLevel level;
    std::wstring message;
    std::wstring source;
};

class ConsoleLogger {
public:
    static ConsoleLogger& GetInstance();

    bool Initialize();

    bool OpenConsole();

    bool CloseConsole();

    void ToggleConsole();

    void ShutdownConsole();

    void SetTitle(const std::wstring& title);

    void SetShowTimestamp(bool show);

    void SetShowLogLevel(bool show);

    void SetShowSource(bool show);

    template<typename... Args>
    void Log(LogLevel level, const std::wstring& source, const std::wstring& message, Args&&... args) {
        wchar_t buffer[1024];
        swprintf_s(buffer, _countof(buffer), message.c_str(), args...);

        LogEntry entry;
        entry.timestamp = std::chrono::system_clock::now();
        entry.level = level;
        entry.message = buffer;
        entry.source = source;

        {
            std::lock_guard<std::mutex> lock(m_queueMutex);
            m_logQueue.push(entry);
        }

        // 将日志同时放入文件写入队列
        {
            std::lock_guard<std::mutex> lock(m_fileQueueMutex);
            m_fileLogQueue.push(entry);
        }
        m_fileQueueCV.notify_one();

        m_queueCV.notify_one();
    }

    template<typename... Args>
    void Debug(const std::wstring& source, const std::wstring& message, Args&&... args) {
        Log(LogLevel::DEBUG, source, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Info(const std::wstring& source, const std::wstring& message, Args&&... args) {
        Log(LogLevel::INFO, source, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Warning(const std::wstring& source, const std::wstring& message, Args&&... args) {
        Log(LogLevel::WARNING, source, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Error(const std::wstring& source, const std::wstring& message, Args&&... args) {
        Log(LogLevel::ERROR, source, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Critical(const std::wstring& source, const std::wstring& message, Args&&... args) {
        Log(LogLevel::CRITICAL, source, message, std::forward<Args>(args)...);
    }

    template<typename... Args>
    void Success(const std::wstring& source, const std::wstring& message, Args&&... args) {
        Log(LogLevel::SUCCESS, source, message, std::forward<Args>(args)...);
    }

    void SetMinLogLevel(LogLevel minLevel);

    void ClearConsole();

    bool SaveToFile(const std::wstring& filePath);

    void SetConsolePosition(int x, int y, int width, int height);

    std::vector<LogEntry> GetLogHistory();

    HWND GetConsoleHandle();

    static void Destroy();

private:
    ConsoleLogger();
    ~ConsoleLogger();
    ConsoleLogger(const ConsoleLogger&) = delete;
    ConsoleLogger& operator=(const ConsoleLogger&) = delete;

    void ConsoleThreadProc();
    void FileWriteThreadProc();  // 新增：文件写入线程
    void ProcessLogQueue();
    void ProcessFileQueue();     // 新增：处理文件写入队列
    void OutputToConsole(const LogEntry& entry);
    bool InitializeLogFile();    // 新增：初始化日志文件

    WORD GetLevelColor(LogLevel level);
    void SetConsoleColor(WORD color);
    void ResetConsoleColor();

    std::wstring FormatTimestamp(const std::chrono::system_clock::time_point& time);
    std::wstring FormatLevel(LogLevel level);
    std::wstring FormatLogEntry(const LogEntry& entry);  // 新增：格式化日志条目

    std::atomic<bool> m_consoleOpen{ false };
    std::atomic<bool> m_initialized{ false };
    std::atomic<bool> m_shutdown{ false };
    std::atomic<LogLevel> m_minLogLevel{ LogLevel::DEBUG };
    std::atomic<bool> m_fileWriteEnabled{ true };  // 新增：是否启用文件写入

    HANDLE m_hConsoleOutput{ INVALID_HANDLE_VALUE };
    HANDLE m_hConsoleInput{ INVALID_HANDLE_VALUE };
    HWND m_hConsoleWnd{ nullptr };

    // 控制台日志队列和同步
    std::mutex m_queueMutex;
    std::condition_variable m_queueCV;
    std::queue<LogEntry> m_logQueue;

    // 文件写入队列和同步
    std::mutex m_fileQueueMutex;
    std::condition_variable m_fileQueueCV;
    std::queue<LogEntry> m_fileLogQueue;

    std::mutex m_historyMutex;
    std::vector<LogEntry> m_logHistory;
    size_t m_maxHistorySize{ 10000 };

    // 文件写入相关
    std::wofstream m_logFile;
    std::wstring m_logFilePath;
    std::mutex m_fileMutex;  // 文件操作互斥锁

    std::thread m_consoleThread;
    std::thread m_fileWriteThread;  // 新增：文件写入线程

    std::atomic<bool> m_showTimestamp{ true };
    std::atomic<bool> m_showLogLevel{ true };
    std::atomic<bool> m_showSource{ true };

    std::wstring m_consoleTitle{ L"Starlight GUI Logger" };

    std::map<LogLevel, WORD> m_colorMap{
        {LogLevel::DEBUG, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_INTENSITY},
        {LogLevel::INFO, FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_INTENSITY},
        {LogLevel::WARNING, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY},
        {LogLevel::ERROR, FOREGROUND_RED | FOREGROUND_INTENSITY},
        {LogLevel::CRITICAL, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY},
        {LogLevel::SUCCESS, FOREGROUND_GREEN | FOREGROUND_INTENSITY}
    };
};