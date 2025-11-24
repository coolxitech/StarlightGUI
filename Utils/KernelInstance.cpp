#include "pch.h"
#include "KernelBase.h"
#include "CppUtils.h"

typedef struct _PROCESS_INPUT {
	ULONG PID;
} PROCESS_INPUT, *PPROCESS_INPUT;

namespace winrt::StarlightGUI::implementation {
	static HANDLE driverDevice = NULL;

	BOOL KernelInstance::_ZwTerminateProcess(DWORD pid) {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		return DeviceIoControl(driverDevice, IOCTL_TERMINATE_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::MurderProcess(DWORD pid) {
        if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		return DeviceIoControl(driverDevice, IOCTL_FORCE_TERMINATE_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_SuspendProcess(DWORD pid) {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		return DeviceIoControl(driverDevice, IOCTL_SUSPEND_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_ResumeProcess(DWORD pid) {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		return DeviceIoControl(driverDevice, IOCTL_RESUME_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::HideProcess(DWORD pid) {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		return DeviceIoControl(driverDevice, IOCTL_HIDE_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::SetPPL(DWORD pid, int level) {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		struct INPUT {
			ULONG PID;
			int level;
		};

		INPUT in = { pid, level };

		return DeviceIoControl(driverDevice, IOCTL_SET_PPL, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::SetCriticalProcess(DWORD pid) {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		return DeviceIoControl(driverDevice, IOCTL_SET_CRITICAL_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_ZwTerminateThread(DWORD tid) {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		return DeviceIoControl(driverDevice, IOCTL_TERMINATE_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::MurderThread(DWORD tid) {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		return DeviceIoControl(driverDevice, IOCTL_FORCE_TERMINATE_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_SuspendThread(DWORD tid) {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		return DeviceIoControl(driverDevice, IOCTL_SUSPEND_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_ResumeThread(DWORD tid) {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		return DeviceIoControl(driverDevice, IOCTL_RESUME_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::EnumProcess(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList) {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		struct INPUT
		{
			ULONG_PTR nSize;
			PVOID ProcessInfo;
		};

		BOOL bRet = FALSE;
		INPUT input = { 0 };

		PDATA_INFO pProcessInfo = NULL;

		input.ProcessInfo = (PVOID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(DATA_INFO) * (targetList.size() + 50));
		input.nSize = sizeof(DATA_INFO) * (targetList.size() + 50);


		ULONG nRet = 0;
		DWORD bytesReturned;
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS, &input, sizeof(INPUT), &nRet, sizeof(ULONG), &bytesReturned, NULL);

		if (status)
		{
			pProcessInfo = (PDATA_INFO)input.ProcessInfo;
			for (ULONG i = 0; i < nRet; i++)
			{
				if (pProcessInfo[i].ulongdata1 != 0)
				{
					auto it = processMap.find((DWORD)pProcessInfo[i].ulongdata1);
					if (it != processMap.end()) {
						winrt::StarlightGUI::ProcessInfo pi = targetList.at(it->second);
						pi.Name(winrt::to_hstring(pProcessInfo[i].Module));
						pi.EProcess(ULongToHexString((ULONG64)pProcessInfo[i].pvoidaddressdata1));
						pi.EProcessULong((ULONG64)pProcessInfo[i].pvoidaddressdata1);
						pi.Status(pProcessInfo[i].ulongdata2 == FALSE ? L"运行中" : L"已隐藏");
					}
					else {
						auto pi = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
						pi.Id(pProcessInfo[i].ulongdata1);
						pi.Name(winrt::to_hstring(pProcessInfo[i].Module));
						pi.Description(L"应用程序");
						pi.ExecutablePath(winrt::to_hstring(pProcessInfo[i].Module1));
						pi.EProcess(ULongToHexString((ULONG64)pProcessInfo[i].pvoidaddressdata1));
						pi.EProcessULong((ULONG64)pProcessInfo[i].pvoidaddressdata1);
						pi.Status(pProcessInfo[i].ulongdata2 == FALSE ? L"运行中" : L"已隐藏");
						targetList.push_back(pi);
					}
				}
				else if (pProcessInfo[i].ulongdata1 == 0) {
					auto pi = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
					pi.Id(pProcessInfo[i].ulongdata1);
					pi.Name(winrt::to_hstring(pProcessInfo[i].Module));
					pi.Description(L"系统");
					pi.ExecutablePath(winrt::to_hstring(pProcessInfo[i].Module1));
					pi.EProcess(ULongToHexString((ULONG64)pProcessInfo[i].pvoidaddressdata1));
					pi.EProcessULong((ULONG64)pProcessInfo[i].pvoidaddressdata1);
					pi.Status(L"系统");
					targetList.push_back(pi);
				}
			}
		}
		bRet = HeapFree(GetProcessHeap(), 0, input.ProcessInfo);
		return status && bRet;
	}

	BOOL KernelInstance::EnumProcessThread(ULONG64 pEprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads)
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;

		struct INPUT
		{
			ULONG nSize;
			ULONG64 pEprocess;
			PDATA_INFO pBuffer;
		};

		INPUT inputs = { 0 };

		ULONG nRetLength = 0;
		inputs.pEprocess = pEprocess;
		inputs.nSize = sizeof(DATA_INFO) * 1000;
		inputs.pBuffer = (DATA_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, inputs.nSize);

		if (inputs.pBuffer)
		{
			bRet = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_THREAD_CIDTABLE, &inputs, sizeof(INPUT), &nRetLength, sizeof(ULONG), 0, 0);
			if (bRet && nRetLength > 0)
			{
				for (ULONG i = 0; i < nRetLength; i++)
				{
					auto threadInfo = winrt::make<winrt::StarlightGUI::implementation::ThreadInfo>();
					threadInfo.Id(inputs.pBuffer[i].ulongdata3);
					threadInfo.EThread(ULongToHexString(inputs.pBuffer[i].ulong64data1));
					threadInfo.Address(ULongToHexString(inputs.pBuffer[i].ulong64data2));
					threadInfo.Priority(inputs.pBuffer[i].ulongdata2);
					threadInfo.ModuleInfo(winrt::to_hstring(inputs.pBuffer[i].Module));
					switch (inputs.pBuffer[i].ulongdata1)
					{
					case ThreadState_Initialized:
						threadInfo.Status(L"初始化");
						break;

					case ThreadState_Ready:
						threadInfo.Status(L"就绪");
						break;

					case ThreadState_Running:
						threadInfo.Status(L"运行中");
						break;

					case ThreadState_Standby:
						threadInfo.Status(L"待命");
						break;

					case ThreadState_Terminated:
						threadInfo.Status(L"已退出");
						break;

					case ThreadState_Waiting:
						threadInfo.Status(L"等待中");
						break;

					case ThreadState_Transition:
						threadInfo.Status(L"Transition");
						break;

					case ThreadState_DeferredReady:
						threadInfo.Status(L"Deferred Ready");
						break;

					case ThreadState_GateWait:
						threadInfo.Status(L"Gate Wait");
						break;

					default:
						threadInfo.Status(L"未知");
						break;
					}
					threads.push_back(threadInfo);
				}
			}

			bRet = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		}
		return bRet;
	}

	BOOL KernelInstance::DisableDSE() {
		return DeviceIoControl(driverDevice, IOCTL_DISABLE_DSE, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableDSE() {
		return DeviceIoControl(driverDevice, IOCTL_ENABLE_DSE, NULL, 0, NULL, 0, NULL, NULL);
	}


	// =================================
	//				PRIVATE
	// =================================

	/*
	* 获取驱动设备位置
	*/
	BOOL KernelInstance::GetDriverDevice() {
		if (driverDevice != NULL) return TRUE;
		if (!DriverUtils::LoadKernelDriver(kernelPath.c_str(), unused)) return FALSE;

		HANDLE device = CreateFile(L"\\\\.\\ArkDrv64", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (device == INVALID_HANDLE_VALUE) return FALSE;

		driverDevice = device;
		return TRUE;
	}

	bool KernelInstance::IsRunningAsAdmin() {
		HANDLE hToken = nullptr;
		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
			return false;
		}

		TOKEN_ELEVATION elevation{};
		DWORD dwSize;
		bool result = GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize);
		CloseHandle(hToken);

		return result && elevation.TokenIsElevated;
	}
}