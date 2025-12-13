#pragma once

#include "pch.h"
#include "IOBase.h"
#include "AstralIO.h"
#include "unordered_set"

namespace winrt::StarlightGUI::implementation {
	class KernelInstance {
	public:
		static bool IsRunningAsAdmin() noexcept;

		// Process
		static BOOL _ZwTerminateProcess(ULONG pid) noexcept;
		static BOOL MurderProcess(ULONG pid) noexcept;

		static BOOL _SuspendProcess(ULONG pid) noexcept;
		static BOOL _ResumeProcess(ULONG pid) noexcept;
		static BOOL HideProcess(ULONG pid) noexcept;
		static BOOL SetPPL(ULONG pid, int level) noexcept;
		static BOOL SetCriticalProcess(ULONG pid) noexcept;
		static BOOL InjectDLLToProcess(ULONG pid, PWCHAR dllPath) noexcept;
		static BOOL ModifyProcessToken(ULONG pid, ULONG type) noexcept;

		// Thread
		static BOOL _ZwTerminateThread(ULONG tid) noexcept;
		static BOOL MurderThread(ULONG tid) noexcept;

		static BOOL _SuspendThread(ULONG tid) noexcept;
		static BOOL _ResumeThread(ULONG tid) noexcept;

		// Driver
		static BOOL UnloadDriver(ULONG64 driverObj) noexcept;
		static BOOL HideDriver(ULONG64 driverObj) noexcept;

		// Enum
		static BOOL EnumProcess(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList) noexcept;
		static BOOL EnumProcessThread(ULONG64 eprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads) noexcept;
		static BOOL EnumProcessHandle(ULONG pid, std::vector<winrt::StarlightGUI::HandleInfo>& handles) noexcept;
		static BOOL EnumProcessModule(ULONG64 eprocess, std::vector<winrt::StarlightGUI::MokuaiInfo>& threads) noexcept;
		static BOOL EnumProcessKernelCallbackTable(ULONG64 eprocess, std::vector<winrt::StarlightGUI::KCTInfo>& threads) noexcept;
		static BOOL EnumDrivers(std::vector<winrt::StarlightGUI::KernelModuleInfo>& kernelModules) noexcept;

		// File
		static BOOL QueryFile(std::wstring path, std::vector<winrt::StarlightGUI::FileInfo>& files) noexcept;
		static BOOL DeleteFileAuto(std::wstring path) noexcept;
		static BOOL _DeleteFileAuto(std::wstring path) noexcept;
		static BOOL MurderFileAuto(std::wstring path) noexcept;
		static BOOL LockFile(std::wstring path) noexcept;
		static BOOL _CopyFile(std::wstring from, std::wstring to, std::wstring name) noexcept;

		// System
		static BOOL EnableCreateProcess() noexcept;
		static BOOL DisableCreateProcess() noexcept;
		static BOOL EnableCreateFile() noexcept;
		static BOOL DisableCreateFile() noexcept;
		static BOOL EnableLoadDriver() noexcept;
		static BOOL DisableLoadDriver() noexcept;
		static BOOL EnableUnloadDriver() noexcept;
		static BOOL DisableUnloadDriver() noexcept;
		static BOOL EnableModifyRegistry() noexcept;
		static BOOL DisableModifyRegistry() noexcept;
		static BOOL ProtectDisk() noexcept;
		static BOOL UnprotectDisk() noexcept;
		static BOOL EnableDSE() noexcept;
		static BOOL DisableDSE() noexcept;
		static BOOL EnableObCallback() noexcept;
		static BOOL DisableObCallback() noexcept;
		static BOOL EnableCmpCallback() noexcept;
		static BOOL DisableCmpCallback() noexcept;
		static BOOL EnableLKD() noexcept;
		static BOOL DisableLKD() noexcept;
		static BOOL Shutdown() noexcept;
		static BOOL Reboot() noexcept;
		static BOOL RebootForce() noexcept;
		static BOOL BlueScreen(ULONG color) noexcept;

	private:
		static BOOL GetDriverDevice() noexcept;
		static BOOL GetDriverDevice2() noexcept;
		static BOOL _DeleteFile(std::wstring path) noexcept;
		static BOOL MurderFile(std::wstring path) noexcept;
	};

	class KernelBase {
	public:
		static DWORD64 GetCIBaseAddress();
		static ULONG64 HackCI();
	};

	class DriverUtils {

	public:
		static bool LoadKernelDriver(LPCWSTR kernelPath, std::wstring& dbgMsg) noexcept;

		static bool LoadDriver(LPCWSTR kernelPath, LPCWSTR fileName, std::wstring& dbgMsg) noexcept;
	};
}