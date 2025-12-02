#include "pch.h"
#include "KernelBase.h"
#include "CppUtils.h"

typedef struct _PROCESS_INPUT {
	ULONG PID;
} PROCESS_INPUT, *PPROCESS_INPUT;

namespace winrt::StarlightGUI::implementation {
	static HANDLE driverDevice = NULL;
	static HANDLE driverDevice2 = NULL;

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
		if (!GetDriverDevice2() || !IsRunningAsAdmin()) return FALSE;


		BOOL bRet = FALSE;
		ENUM_PROCESS input = { 0 };

		PPROCESS_DATA pProcessInfo = NULL;

		input.Buffer = (PVOID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(PROCESS_DATA) * (targetList.size() + 50));
		input.BufferSize = sizeof(PROCESS_DATA) * (targetList.size() + 50);
		input.ProcessCount = 0;

		BOOL status;

		status = DeviceIoControl(driverDevice2, IOCTL_AX_ENUM_PROCESSES, &input, sizeof(ENUM_PROCESS), &input, sizeof(ENUM_PROCESS), 0, NULL);

		if (status)
		{
			pProcessInfo = (PPROCESS_DATA)input.Buffer;
			for (ULONG i = 0; i < input.ProcessCount; i++)
			{
				PROCESS_DATA data = pProcessInfo[i];
				if (data.Pid != 0)
				{
					auto it = processMap.find((DWORD)data.Pid);
					if (it != processMap.end()) {
						winrt::StarlightGUI::ProcessInfo pi = targetList.at(it->second);
						pi.Name(winrt::to_hstring(data.ImageName));
						pi.EProcess(ULongToHexString((ULONG64)data.Eprocess));
						pi.EProcessULong((ULONG64)data.Eprocess);
						pi.MemoryUsageByte(data.WorkingSetPrivateSize);
					}
					else {
						auto pi = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
						pi.Id(data.Pid);
						pi.Name(winrt::to_hstring(data.ImageName));
						pi.EProcess(ULongToHexString((ULONG64)data.Eprocess));
						pi.EProcessULong((ULONG64)data.Eprocess);
						pi.Description(L"应用程序");
						pi.ExecutablePath(winrt::to_hstring(data.ImagePath));
						pi.MemoryUsageByte(data.WorkingSetPrivateSize);
						targetList.push_back(pi);
					}
				}
				else if (data.Pid == 0) {
					auto pi = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
					pi.Id(data.Pid);
					pi.Name(winrt::to_hstring(data.ImageName));
					pi.Description(L"系统");
					pi.ExecutablePath(winrt::to_hstring(data.ImagePath));
					pi.EProcess(ULongToHexString(data.Eprocess));
					pi.EProcessULong((ULONG64)data.Eprocess);
					pi.Status(L"系统");
					pi.MemoryUsageByte(data.WorkingSetPrivateSize);
					targetList.push_back(pi);
				}
			}
		}
		bRet = HeapFree(GetProcessHeap(), 0, input.Buffer);
		return status && bRet;
	}

	BOOL KernelInstance::EnumProcessThread(ULONG64 eprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads)
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL status = FALSE;

		struct INPUT
		{
			ULONG nSize;
			ULONG64 pEprocess;
			PDATA_INFO pBuffer;
		};

		INPUT inputs = { 0 };

		ULONG nRet = 0;
		inputs.pEprocess = eprocess;
		inputs.nSize = sizeof(DATA_INFO) * 1000;
		inputs.pBuffer = (PDATA_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, inputs.nSize);

		status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_THREAD_CIDTABLE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) {
			nRet = 1000;
		}

		if (status && nRet > 0 && inputs.pBuffer)
		{
			for (ULONG i = 0; i < nRet; i++)
			{
				DATA_INFO data = inputs.pBuffer[i];
				auto threadInfo = winrt::make<winrt::StarlightGUI::implementation::ThreadInfo>();
				threadInfo.Id(data.ulongdata3);
				threadInfo.EThread(ULongToHexString(data.ulong64data1));
				threadInfo.Address(ULongToHexString(data.ulong64data2));
				threadInfo.Priority(data.ulongdata2);
				threadInfo.ModuleInfo(winrt::to_hstring(data.Module));
				switch (data.ulongdata1)
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

		status = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		return status;
	}

	BOOL KernelInstance::EnumProcessHandle(ULONG pid, std::vector<winrt::StarlightGUI::HandleInfo>& handles)
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;

		struct INPUT
		{
			ULONG nSize;
			ULONG PID;
			PDATA_INFO pBuffer;
		};
		PDATA_INFO pProcessInfo = NULL;
		INPUT inputs = { 0 };

		inputs.nSize = sizeof(DATA_INFO) * 1000;
		inputs.pBuffer = (PDATA_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, inputs.nSize);
		inputs.PID = pid;

		ULONG nRet = 0;
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_EXIST_HANDLE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) {
			nRet = 1000;
		}

		if (status && nRet > 0 && inputs.pBuffer) {
			pProcessInfo = (PDATA_INFO)inputs.pBuffer;
			for (ULONG i = 0; i < nRet; i++)
			{
				DATA_INFO data = pProcessInfo[i];
				auto handleInfo = winrt::make<winrt::StarlightGUI::implementation::HandleInfo>();
				handleInfo.Type(to_hstring(data.Module));
				handleInfo.Object(ULongToHexString((ULONG64)data.pvoidaddressdata1));
				handleInfo.Handle(ULongToHexString(data.ulong64data3));
				handleInfo.Access(ULongToHexString(data.ulong64data1, 0, false, true));
				handleInfo.Attributes(ULongToHexString(data.ulong64data2, 0, false, true));
				handles.push_back(handleInfo);
			}
		}

		OutputDebugString(std::to_wstring(nRet).c_str());

		bRet = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		return bRet;
	}

	BOOL KernelInstance::EnumProcessModule(ULONG64 eprocess, std::vector<winrt::StarlightGUI::MokuaiInfo>& modules)
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;

		struct INPUT
		{
			ULONG nSize;
			PVOID eproc;
			PDATA_INFO pBuffer;
		};
		PDATA_INFO pProcessInfo = NULL;
		INPUT inputs = { 0 };

		inputs.nSize = sizeof(DATA_INFO) * 1000;
		inputs.pBuffer = (PDATA_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, inputs.nSize);
		inputs.eproc = (PVOID)eprocess;

		ULONG nRet = 0;
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_MODULE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) {
			inputs.nSize = sizeof(DATA_INFO) * nRet;
			inputs.pBuffer = (PDATA_INFO)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, inputs.nSize);
			status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_MODULE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);
			nRet = 1000;
		}

		if (status && nRet > 0 && inputs.pBuffer) {
			pProcessInfo = (PDATA_INFO)inputs.pBuffer;
			if (nRet >= 1)
			{
				for (ULONG i = 0; i < nRet; i++)
				{
					DATA_INFO data = pProcessInfo[i];
					auto moduleInfo = winrt::make<winrt::StarlightGUI::implementation::MokuaiInfo>();
					moduleInfo.Name(to_hstring(data.Module));
					moduleInfo.Address(ULongToHexString(data.ulong64data1));
					moduleInfo.Size(ULongToHexString(data.ulong64data2, 0, false, true));
					moduleInfo.Path(to_hstring(data.Module1));
					modules.push_back(moduleInfo);
				}
			}
		}

		bRet = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		return bRet;
	}

	BOOL KernelInstance::DisableDSE() {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		return DeviceIoControl(driverDevice, IOCTL_DISABLE_DSE, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableDSE() {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
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

	BOOL KernelInstance::GetDriverDevice2() {
		if (driverDevice2 != NULL) return TRUE;

		HANDLE device = CreateFile(L"\\\\.\\AstralX", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (device == INVALID_HANDLE_VALUE) return FALSE;

		driverDevice2 = device;
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