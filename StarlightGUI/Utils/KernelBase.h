#pragma once

#include "pch.h"
#include "IOBase.h"
#include "unordered_set"

namespace winrt::StarlightGUI::implementation {
	class KernelInstance {
	public:
		static bool IsRunningAsAdmin();

		// Process
		static BOOL _ZwTerminateProcess(DWORD pid);
		static BOOL MurderProcess(DWORD pid);

		static BOOL _SuspendProcess(DWORD pid);
		static BOOL _ResumeProcess(DWORD pid);
		static BOOL HideProcess(DWORD pid);
		static BOOL SetPPL(DWORD pid, int level);
		static BOOL SetCriticalProcess(DWORD pid);

		// Thread
		static BOOL _ZwTerminateThread(DWORD tid);
		static BOOL MurderThread(DWORD tid);

		static BOOL _SuspendThread(DWORD tid);
		static BOOL _ResumeThread(DWORD tid);

		// Enum
		static BOOL EnumProcess(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList);
		static BOOL EnumProcessThread(ULONG64 eprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads);

		// System
		static BOOL EnableDSE();
		static BOOL DisableDSE();

	private:
		static BOOL GetDriverDevice();
	};

	class KernelBase {
	public:
		static DWORD64 GetCIBaseAddress();
		static ULONG64 HackCI();
	};

	class DriverUtils {

	public:
		static bool LoadKernelDriver(LPCWSTR kernelPath, std::wstring& dbgMsg);

		static bool LoadDriver(LPCWSTR kernelPath, LPCWSTR fileName, std::wstring& dbgMsg);
	};
}