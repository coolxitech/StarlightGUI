#pragma once

#include "pch.h"
#include "IOBase.h"
#include "AstralIO.h"
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
		static BOOL InjectDLLToProcess(DWORD pid, PWCHAR dllPath);

		// Thread
		static BOOL _ZwTerminateThread(DWORD tid);
		static BOOL MurderThread(DWORD tid);

		static BOOL _SuspendThread(DWORD tid);
		static BOOL _ResumeThread(DWORD tid);

		// Driver
		static BOOL UnloadDriver(ULONG64 driverObj);
		static BOOL HideDriver(ULONG64 driverObj);

		// Enum
		static BOOL EnumProcess(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList);
		static BOOL EnumProcessThread(ULONG64 eprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads);
		static BOOL EnumProcessHandle(ULONG pid, std::vector<winrt::StarlightGUI::HandleInfo>& handles);
		static BOOL EnumProcessModule(ULONG64 eprocess, std::vector<winrt::StarlightGUI::MokuaiInfo>& threads);
		static BOOL EnumProcessKernelCallbackTable(ULONG64 eprocess, std::vector<winrt::StarlightGUI::KCTInfo>& threads);
		static BOOL EnumDrivers(std::vector<winrt::StarlightGUI::KernelModuleInfo>& kernelModules);

		// File
		static BOOL QueryFile(std::wstring path, std::vector<winrt::StarlightGUI::FileInfo>& files);

		// System
		static BOOL EnableDSE();
		static BOOL DisableDSE();

	private:
		static BOOL GetDriverDevice();
		static BOOL GetDriverDevice2();
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