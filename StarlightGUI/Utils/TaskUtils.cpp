#include "pch.h"
#include "TaskUtils.h"
#include "Utils.h"
#include "TlHelp32.h"
#include "KernelBase.h"
#include "shellapi.h"
#include "Psapi.h"

typedef BOOL(WINAPI* P_EndTask)(HWND hwnd, BOOL fShutdown, BOOL fForce);
typedef NTSTATUS(NTAPI* P_NtQuerySystemInformation)(_SYSTEM_INFORMATION_CLASS SystemInformationClass, PVOID SystemInformation, ULONG SystemInformationLength, PULONG ReturnLength);

P_EndTask _EndTask{ nullptr };
P_NtQuerySystemInformation _NtQuerySystemInformation{ nullptr };

namespace winrt::StarlightGUI::implementation {
	void TaskUtils::EnsurePrivileges() {
		TaskUtils::EnableDebugPrivilege();
	}

	bool TaskUtils::_TerminateProcess(DWORD pid) {
		if (pid != 0) {
            HANDLE hProc = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
			if (hProc) {
				return TerminateProcess(hProc, 0);
			}
		}
		return false;
	}

	bool TaskUtils::_TerminateThread(DWORD tid) {
		if (tid != 0) {
			HANDLE hThread = OpenThread(THREAD_TERMINATE, FALSE, tid);
			if (hThread) {
				return TerminateThread(hThread, 0);
			}
		}
		return false;
	}

	/*
	* 开启进程效能模式
	*/
	bool TaskUtils::EnableProcessPerformanceMode(StarlightGUI::ProcessInfo pi) {
		int pid = pi.Id();
		if (pid != 0) {
			HANDLE hProc = OpenProcess(PROCESS_SET_INFORMATION, FALSE, pid);

			if (hProc) {
				PROCESS_POWER_THROTTLING_STATE throttling;
				ZeroMemory(&throttling, sizeof(throttling));

				throttling.Version = PROCESS_POWER_THROTTLING_CURRENT_VERSION;
				throttling.ControlMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;
				throttling.StateMask = PROCESS_POWER_THROTTLING_EXECUTION_SPEED;

				BOOL result = SetProcessInformation(hProc, ProcessPowerThrottling, &throttling, sizeof(throttling));
				CloseHandle(hProc);

				return result;
			}
		}
		return false;
	}

	/*
	* 获取进程私有工作集
	*/
	SIZE_T TaskUtils::GetProcessWorkingSet(HANDLE hProc) {
		PSAPI_WORKING_SET_INFORMATION workSetInfo{};
		PBYTE pByte = NULL;
		PSAPI_WORKING_SET_BLOCK* pWorkSetBlock = workSetInfo.WorkingSetInfo;

		if (!K32QueryWorkingSet(hProc, &workSetInfo, sizeof(workSetInfo))) {
			if (GetLastError() == ERROR_BAD_LENGTH)
			{
				DWORD realSize = sizeof(workSetInfo.NumberOfEntries)
					+ workSetInfo.NumberOfEntries * sizeof(PSAPI_WORKING_SET_BLOCK);
				try
				{
					pByte = new BYTE[realSize];
					memset(pByte, 0, realSize);
					pWorkSetBlock = (PSAPI_WORKING_SET_BLOCK*)(pByte + sizeof(workSetInfo.NumberOfEntries));
					if (!K32QueryWorkingSet(hProc, pByte, realSize))
					{
						delete[] pByte;
						return 0;
					}
				}
				catch (char* e)
				{
					return 0;
				}
			}
			else {
				return 0;
			}
		}
		PERFORMANCE_INFORMATION performanceInfo{};
		if (!K32GetPerformanceInfo(&performanceInfo, sizeof(performanceInfo))) return 0;
		SIZE_T pageSize = performanceInfo.PageSize;
		SIZE_T privateWorkingSet = 0;
		for (ULONG_PTR i = 0; i < workSetInfo.NumberOfEntries; ++i)
		{
			if (!pWorkSetBlock[i].Shared) // Remove shared pages
				privateWorkingSet += pageSize;
		}
		if (pByte)
			delete[] pByte;
		return privateWorkingSet;
	}

	/*
	* 获取进程CPU占用
	*/
	winrt::Windows::Foundation::IAsyncAction TaskUtils::FetchProcessCpuUsage(std::map<DWORD, hstring>& processCpuTable) {
		co_await winrt::resume_background();

		if (!_NtQuerySystemInformation) {
			HMODULE hNtdll = LoadLibraryW(L"ntdll.dll");
			_NtQuerySystemInformation = (P_NtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
		}

		if (!_NtQuerySystemInformation) co_return; // Check again to ensure safety

		ULONG len = 0;
		std::vector<BYTE> buffer(0x10000);
		NTSTATUS status = _NtQuerySystemInformation(SystemProcessInformation, buffer.data(), len, &len);

		if (NT_ERROR(status)) {
			buffer.resize(len);
			status = _NtQuerySystemInformation(SystemProcessInformation, buffer.data(), len, &len);
		}

		if (!NT_SUCCESS(status)) co_return;

		// Loop through the processes and fill the map with PID and CPU usage
		PMY_SYSTEM_PROCESS_INFORMATION spi = (PMY_SYSTEM_PROCESS_INFORMATION)buffer.data();
		while (spi) {
			co_await TaskUtils::ReadSPIAsync(spi, processCpuTable);

			if (spi->NextEntryOffset == 0)
				break;
			spi = (PMY_SYSTEM_PROCESS_INFORMATION)((BYTE*)spi + spi->NextEntryOffset);
		}

		co_return;
	}

	winrt::Windows::Foundation::IAsyncAction TaskUtils::ReadSPIAsync(PMY_SYSTEM_PROCESS_INFORMATION& spi, std::map<DWORD, hstring>& processCpuTable) {
		// Loop through the processes and fill the map with PID and CPU usage
		LONGLONG cpuUsage = spi->UserTime.QuadPart + spi->KernelTime.QuadPart;
		if (cpuUsage > 0) {
			processCpuTable[(DWORD)(ULONG_PTR)spi->UniqueProcessId] = to_hstring(std::round(cpuUsage / 1000000000.0) / 10.0) + L"s";
		}
		else processCpuTable[(DWORD)(ULONG_PTR)spi->UniqueProcessId] = L"0s";

		co_return;
	}

	/*
	* 复制至剪贴板
	*/
	bool TaskUtils::CopyToClipboard(std::wstring str) {
		if (str.empty()) {
			return false;
		}

		if (!OpenClipboard(nullptr)) {
			return false;
		}

		if (!EmptyClipboard()) {
			CloseClipboard();
			return false;
		}

		size_t sizeInBytes = (str.size() + 1) * sizeof(wchar_t);
		HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, sizeInBytes);
		if (!hGlobal) {
			CloseClipboard();
			return false;
		}

		void* pGlobal = GlobalLock(hGlobal);
		if (!pGlobal) {
			GlobalFree(hGlobal);
			CloseClipboard();
			return false;
		}
		memcpy(pGlobal, str.c_str(), sizeInBytes);
		GlobalUnlock(hGlobal);

		if (!SetClipboardData(CF_UNICODETEXT, hGlobal)) {
			GlobalFree(hGlobal);
			CloseClipboard();
			return false;
		}

		CloseClipboard();
		return true;
	}
	/*
	* 打开文件所在位置
	*/
	bool TaskUtils::OpenFolderAndSelectFile(std::wstring filePath) {
		PIDLIST_ABSOLUTE pidl = nullptr;

		HRESULT hr = SHParseDisplayName(filePath.c_str(), nullptr, &pidl, 0, nullptr);
		if (FAILED(hr) || !pidl) {
			return false;
			return false;
		}

		hr = SHOpenFolderAndSelectItems(pidl, 0, nullptr, 0);
		CoTaskMemFree(pidl);

		if (FAILED(hr)) {
			return false;
		}
		return true;
	}

	/*
	* 打开文件属性
	*/
	bool TaskUtils::OpenFileProperties(std::wstring filePath) {
		SHELLEXECUTEINFOW sei = { 0 };
		sei.cbSize = sizeof(sei);
		sei.fMask = SEE_MASK_INVOKEIDLIST;
		sei.hwnd = NULL;
		sei.lpVerb = L"properties";
		sei.lpFile = filePath.c_str();
		sei.nShow = SW_SHOW;

		return ShellExecuteExW(&sei) != FALSE;
	}

	// =====================================================
	// Private --- Starting here
	// =====================================================
	BOOL CALLBACK TaskUtils::EndTaskByWindow(HWND hwnd, LPARAM lparam) {
		DWORD* procPid = (DWORD*)lparam;
		DWORD windowPid;

		if (_EndTask == nullptr) {
			_EndTask = (P_EndTask)GetProcAddress(GetModuleHandleW(L"user32.dll"), "EndTask");
		}

		// Get thread ID
		GetWindowThreadProcessId(hwnd, &windowPid);

		if (windowPid == *procPid) {
			_EndTask(hwnd, FALSE, TRUE);
		}
		return TRUE;
	}

	bool TaskUtils::EnableDebugPrivilege() {
		HANDLE hToken;
		TOKEN_PRIVILEGES tkp;

		if (!OpenProcessToken(GetCurrentProcess(),
			TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
			return false;
		}

		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);

		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

		BOOL result = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, NULL, 0);
		CloseHandle(hToken);

		return result != FALSE;
	}
}