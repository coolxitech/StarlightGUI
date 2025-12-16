#include "pch.h"
#include "KernelBase.h"
#include "CppUtils.h"

typedef struct _PROCESS_INPUT {
	ULONG PID;
} PROCESS_INPUT, *PPROCESS_INPUT;

typedef struct _DRIVER_INPUT {
	PVOID DriverObj;
} DRIVER_INPUT, * PDRIVER_INPUT;

namespace winrt::StarlightGUI::implementation {
	static HANDLE driverDevice = NULL;
	static HANDLE driverDevice2 = NULL;

	BOOL KernelInstance::_ZwTerminateProcess(ULONG pid) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_TERMINATE_PROCESS, __WFUNCTION__.c_str(), pid);
		return DeviceIoControl(driverDevice, IOCTL_TERMINATE_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::MurderProcess(ULONG pid) noexcept {
        if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_FORCE_TERMINATE_PROCESS, __WFUNCTION__.c_str(), pid);
		return DeviceIoControl(driverDevice, IOCTL_FORCE_TERMINATE_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_SuspendProcess(ULONG pid) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_SUSPEND_PROCESS, __WFUNCTION__.c_str(), pid);
		return DeviceIoControl(driverDevice, IOCTL_SUSPEND_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_ResumeProcess(ULONG pid) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_RESUME_PROCESS, __WFUNCTION__.c_str(), pid);
		return DeviceIoControl(driverDevice, IOCTL_RESUME_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::HideProcess(ULONG pid) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_HIDE_PROCESS, __WFUNCTION__.c_str(), pid);
		return DeviceIoControl(driverDevice, IOCTL_HIDE_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::SetPPL(ULONG pid, int level) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		struct INPUT {
			ULONG PID;
			int level;
		};

		INPUT in = { pid, level };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d, level=%d]", IOCTL_TERMINATE_PROCESS, __WFUNCTION__.c_str(), pid, level);
		return DeviceIoControl(driverDevice, IOCTL_SET_PPL, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::SetCriticalProcess(ULONG pid) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { pid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_SET_CRITICAL_PROCESS, __WFUNCTION__.c_str(), pid);
		return DeviceIoControl(driverDevice, IOCTL_SET_CRITICAL_PROCESS, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::InjectDLLToProcess(ULONG pid, PWCHAR dllPath) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		struct INPUT {
			ULONG PID;
			UNICODE_STRING DllPath[MAX_PATH];
		};

		INPUT in = { 0 };
		in.PID = pid;
		RtlInitUnicodeString(in.DllPath, dllPath);

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d, dllPath=%s]", IOCTL_SHELLCODE_INJECT_DLL, __WFUNCTION__.c_str(), pid, dllPath);
		return DeviceIoControl(driverDevice, IOCTL_SHELLCODE_INJECT_DLL, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::ModifyProcessToken(ULONG pid, ULONG type) noexcept {
		if (pid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		struct INPUT {
			ULONG PID;
			ULONG Type;
		};

		INPUT in = { 0 };
		in.PID = pid;
		in.Type = type;

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d, type=%d]", IOCTL_MODIFY_PROCESS_TOKEN, __WFUNCTION__.c_str(), pid, type);
		return DeviceIoControl(driverDevice, IOCTL_MODIFY_PROCESS_TOKEN, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_ZwTerminateThread(ULONG tid) noexcept {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [tid=%d]", IOCTL_TERMINATE_THREAD, __WFUNCTION__.c_str(), tid);
		return DeviceIoControl(driverDevice, IOCTL_TERMINATE_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::MurderThread(ULONG tid) noexcept {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [tid=%d]", IOCTL_FORCE_TERMINATE_THREAD, __WFUNCTION__.c_str(), tid);
		return DeviceIoControl(driverDevice, IOCTL_FORCE_TERMINATE_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_SuspendThread(ULONG tid) noexcept {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [tid=%d]", IOCTL_SUSPEND_THREAD, __WFUNCTION__.c_str(), tid);
		return DeviceIoControl(driverDevice, IOCTL_SUSPEND_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::_ResumeThread(ULONG tid) noexcept {
		if (tid == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		PROCESS_INPUT in = { tid };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [tid=%d]", IOCTL_RESUME_THREAD, __WFUNCTION__.c_str(), tid);
		return DeviceIoControl(driverDevice, IOCTL_RESUME_THREAD, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::UnloadDriver(ULONG64 driverObj) noexcept {
		if (driverObj == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		DRIVER_INPUT in = { (PVOID)driverObj };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [driverObj=0x%X]", IOCTL_UNLOAD_DRIVER, __WFUNCTION__.c_str(), driverObj);
		return DeviceIoControl(driverDevice, IOCTL_UNLOAD_DRIVER, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::HideDriver(ULONG64 driverObj) noexcept {
		if (driverObj == 0) return FALSE;
		if (!GetDriverDevice()) return FALSE;

		DRIVER_INPUT in = { (PVOID)driverObj };

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [driverObj=0x%X]", IOCTL_HIDE_DRIVER, __WFUNCTION__.c_str(), driverObj);
		return DeviceIoControl(driverDevice, IOCTL_HIDE_DRIVER, &in, sizeof(in), 0, 0, 0, NULL);
	}

	BOOL KernelInstance::EnumProcess(std::unordered_map<DWORD, int> processMap, std::vector<winrt::StarlightGUI::ProcessInfo>& targetList) noexcept {
		if (!GetDriverDevice2() || !IsRunningAsAdmin()) return FALSE;

		BOOL bRet = FALSE;
		ENUM_PROCESS input = { 0 };

		PPROCESS_DATA pProcessInfo = NULL;

		input.BufferSize = sizeof(PROCESS_DATA) * (targetList.size() + 100);
		input.Buffer = (PVOID)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, input.BufferSize);
		input.ProcessCount = 0;

		BOOL status;

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_AX_ENUM_PROCESSES, __WFUNCTION__.c_str());
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
						pi.Name(to_hstring(data.ImageName));
						pi.EProcess(ULongToHexString((ULONG64)data.Eprocess));
						pi.EProcessULong((ULONG64)data.Eprocess);
						pi.MemoryUsageByte(data.WorkingSetPrivateSize);
					}
					else {
						auto pi = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
						pi.Id(data.Pid);
						pi.Name(to_hstring(data.ImageName));
						pi.EProcess(ULongToHexString((ULONG64)data.Eprocess));
						pi.EProcessULong((ULONG64)data.Eprocess);
						pi.Description(L"应用程序");
						pi.ExecutablePath(to_hstring(data.ImagePath));
						pi.MemoryUsageByte(data.WorkingSetPrivateSize);
						targetList.push_back(pi);
					}
				}
				else if (data.Pid == 0) {
					auto pi = winrt::make<winrt::StarlightGUI::implementation::ProcessInfo>();
					pi.Id(data.Pid);
					pi.Name(to_hstring(data.ImageName));
					pi.Description(L"系统");
					pi.ExecutablePath(to_hstring(data.ImagePath));
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

	BOOL KernelInstance::EnumProcessThread(ULONG64 eprocess, std::vector<winrt::StarlightGUI::ThreadInfo>& threads) noexcept
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

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [eprocess=0x%X]", IOCTL_ENUM_PROCESS_THREAD_CIDTABLE, __WFUNCTION__.c_str(), eprocess);
		status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_THREAD_CIDTABLE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) nRet = 1000;

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

	BOOL KernelInstance::EnumProcessHandle(ULONG pid, std::vector<winrt::StarlightGUI::HandleInfo>& handles) noexcept
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;
		ULONG nRet = 0;

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

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [pid=%d]", IOCTL_ENUM_PROCESS_EXIST_HANDLE, __WFUNCTION__.c_str(), pid);
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_EXIST_HANDLE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) nRet = 1000;

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

	BOOL KernelInstance::EnumProcessModule(ULONG64 eprocess, std::vector<winrt::StarlightGUI::MokuaiInfo>& modules) noexcept
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;
		ULONG nRet = 0;

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

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [eprocess=0x%X]", IOCTL_ENUM_PROCESS_MODULE, __WFUNCTION__.c_str(), eprocess);
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_PROCESS_MODULE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) nRet = 1000;

		if (status && nRet > 0 && inputs.pBuffer) {
			pProcessInfo = (PDATA_INFO)inputs.pBuffer;
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

		bRet = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		return bRet;
	}

	BOOL KernelInstance::EnumProcessKernelCallbackTable(ULONG64 eprocess, std::vector<winrt::StarlightGUI::KCTInfo>& modules) noexcept
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;
		ULONG nRet = 0;

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

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [eprocess=0x%X]", IOCTL_ENUM_KERNELCALLBACKTABLE, __WFUNCTION__.c_str(), eprocess);
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_KERNELCALLBACKTABLE, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, NULL);

		if (nRet > 1000) nRet = 1000;

		if (status && nRet > 0 && inputs.pBuffer) {
			pProcessInfo = (PDATA_INFO)inputs.pBuffer;
			for (ULONG i = 0; i < nRet; i++)
			{
				DATA_INFO data = pProcessInfo[i];
				auto kctInfo = winrt::make<winrt::StarlightGUI::implementation::KCTInfo>();
				kctInfo.Name(to_hstring(data.Module));
				kctInfo.Address(ULongToHexString(data.ulong64data1));
				modules.push_back(kctInfo);
			}
		}

		bRet = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		return bRet;
	}

	BOOL KernelInstance::EnumDrivers(std::vector<winrt::StarlightGUI::KernelModuleInfo>& kernelModules) noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		struct INPUT {
			ULONG nSize;
			PALL_DRIVERS pBuffer;
		};

		BOOL bRet = FALSE;
		INPUT input = { 0 };

		PPROCESS_DATA pProcessInfo = NULL;

		input.nSize = sizeof(ALL_DRIVERS) * 1000;
		input.pBuffer = (PALL_DRIVERS)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, input.nSize);

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_ENUM_DRIVERS, __WFUNCTION__.c_str());
		BOOL status = DeviceIoControl(driverDevice, IOCTL_ENUM_DRIVERS, &input, sizeof(INPUT), 0, 0, 0, NULL);

		if (status && input.pBuffer && input.pBuffer->nCnt > 0)
		{
			for (ULONG i = 0; i < input.pBuffer->nCnt; i++)
			{
				DRIVER_INFO data = input.pBuffer->Drivers[i];
				auto di = winrt::make<winrt::StarlightGUI::implementation::KernelModuleInfo>();
				di.Name(to_hstring(data.szDriverName));
				di.Path(to_hstring(data.szDriverPath));
				di.ImageBase(ULongToHexString(data.nBase));
				di.ImageBaseULong(data.nBase);
				di.Size(ULongToHexString(data.nSize, 0, false, true));
				di.SizeULong(data.nSize);
				di.LoadOrder(ULongToHexString(data.nLoadOrder, 0, false, true));
				di.LoadOrderULong(data.nLoadOrder);
				di.DriverObject(ULongToHexString(data.nDriverObject));
				di.DriverObjectULong(data.nDriverObject);
				kernelModules.push_back(di);
			}
		}
		bRet = HeapFree(GetProcessHeap(), 0, input.pBuffer);
		return status && bRet;
	}

	BOOL KernelInstance::QueryFile(std::wstring path, std::vector<winrt::StarlightGUI::FileInfo>& files) noexcept
	{
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		BOOL bRet = FALSE;
		ULONG nRet = 0;
		std::string enumFileMode = ReadConfig("enum_file_mode", "ENUM_FILE_NTAPI");

		struct INPUT
		{
			ULONG_PTR nSize;
			PDATA_INFO pBuffer;
			UNICODE_STRING path[MAX_PATH];
		};

		LOG_WARNING(L"KernelInstance", L"Enum file mode: %s", to_hstring(enumFileMode).c_str());

		INPUT inputs = { 0 };

		if (enumFileMode == "ENUM_FILE_IRP")
		{
			RtlInitUnicodeString(inputs.path, path.c_str());
		}
		else
		{
			WCHAR targetPath[MAX_PATH];
			wcscpy_s(targetPath, L"\\??\\");
			wcscat_s(targetPath, path.c_str());
			RtlInitUnicodeString(inputs.path, targetPath);
		}

		inputs.nSize = sizeof(DATA_INFO) * 10000;
		inputs.pBuffer = (DATA_INFO*)HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, inputs.nSize);
		if (enumFileMode == "ENUM_FILE_NTFSPARSER")
		{
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [path=%s]", IOCTL_NTFS_PARSER_ENUM_FILE2, __WFUNCTION__.c_str(), path.c_str());
			bRet = DeviceIoControl(driverDevice, IOCTL_NTFS_PARSER_ENUM_FILE2, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, 0);
			if (bRet && nRet > 0 && nRet < 10000)
			{
				std::vector<winrt::StarlightGUI::FileInfo> result;
				for (ULONG i = 0; i < nRet; i++)
				{
					DATA_INFO data = inputs.pBuffer[i];
					auto fileInfo = winrt::make<winrt::StarlightGUI::implementation::FileInfo>();
					fileInfo.Name(data.wcstr);
					fileInfo.Path(path + L"\\" + data.wcstr);
					fileInfo.Flag(data.ulongdata1);
					fileInfo.Directory(data.ulongdata1 != MFT_RECORD_FLAG_FILE);
					fileInfo.Size(FormatMemorySize(data.ulong64data2));
					fileInfo.SizeULong(data.ulong64data2);
					fileInfo.MFTID(data.ulong64data1);
					result.push_back(fileInfo);
				}
				// Remove duplicated MFT indexes
				std::unordered_map<ULONG64, size_t> keep;
				for (size_t i = 0; i < result.size(); ++i) {
					ULONG64 mft = result[i].MFTID();
					auto it = keep.find(mft);

					if (it == keep.end()) keep[mft] = i;
					else {
						std::wstring_view curr = result[i].Name(), kept = result[it->second].Name();
						bool cHas = curr.find(L'~') != std::wstring_view::npos;
						bool kHas = kept.find(L'~') != std::wstring_view::npos;

						if ((!cHas && kHas) || (cHas == kHas && curr.length() < kept.length())) {
							it->second = i;
						}
					}
				}

				result.reserve(keep.size());
				for (auto& [mft, idx] : keep) files.push_back(std::move(result[idx]));
			}
		}
		else if (enumFileMode == "ENUM_FILE_NTAPI")
		{
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [path=%s]", IOCTL_QUERY_FILE2, __WFUNCTION__.c_str(), path.c_str());
			bRet = DeviceIoControl(driverDevice, IOCTL_QUERY_FILE2, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, 0);
			if (bRet && nRet > 0 && nRet < 10000)
			{
				for (ULONG i = 0; i < nRet; i++)
				{
					DATA_INFO data = inputs.pBuffer[i];
					auto fileInfo = winrt::make<winrt::StarlightGUI::implementation::FileInfo>();
					fileInfo.Name(data.wcstr);
					fileInfo.Path(path + L"\\" + data.wcstr);
					if (data.ulongdata4 == FALSE)
					{
						fileInfo.Flag(MFT_RECORD_FLAG_FILE);
						fileInfo.Directory(false);
					}
					else
					{
						fileInfo.Flag(MFT_RECORD_FLAG_DIRECTORY);
						fileInfo.Directory(true);
					}
					files.push_back(fileInfo);
				}
			}
		}
		else if (enumFileMode == "ENUM_FILE_IRP")
		{
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [path=%s]", IOCTL_QUERY_FILE_IRP, __WFUNCTION__.c_str(), path.c_str());
			bRet = DeviceIoControl(driverDevice, IOCTL_QUERY_FILE_IRP, &inputs, sizeof(INPUT), &nRet, sizeof(ULONG), 0, 0);
			if (bRet && nRet > 0 && nRet < 10000)
			{
				for (ULONG i = 0; i < nRet; i++)
				{
					DATA_INFO data = inputs.pBuffer[i];
					auto fileInfo = winrt::make<winrt::StarlightGUI::implementation::FileInfo>();
					fileInfo.Name(data.wcstr);
					fileInfo.Path(path + L"\\" + data.wcstr);
					if (data.ulongdata4 == FALSE)
					{
						fileInfo.Flag(MFT_RECORD_FLAG_FILE);
						fileInfo.Directory(false);
					}
					else
					{
						fileInfo.Flag(MFT_RECORD_FLAG_DIRECTORY);
						fileInfo.Directory(true);
					}
					files.push_back(fileInfo);
				}
			}
		}
		bRet = HeapFree(GetProcessHeap(), 0, inputs.pBuffer);
		return bRet;
	}

	BOOL KernelInstance::_DeleteFile(std::wstring path) noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		WCHAR targetPath[MAX_PATH];
		wcscpy_s(targetPath, L"\\??\\");
		wcscat_s(targetPath, path.c_str());

		UNICODE_STRING filePath[MAX_PATH];
		RtlInitUnicodeString(filePath, targetPath);

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [path=%s]", IOCTL_DELETE_FILE_UNICODE, __WFUNCTION__.c_str(), path.c_str());
		return DeviceIoControl(driverDevice, IOCTL_DELETE_FILE_UNICODE, filePath, sizeof(filePath), NULL, 0, 0, NULL);
	}

	BOOL KernelInstance::MurderFile(std::wstring path) noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		WCHAR targetPath[MAX_PATH];
		wcscpy_s(targetPath, L"\\??\\");
		wcscat_s(targetPath, path.c_str());

		UNICODE_STRING filePath[MAX_PATH];
		RtlInitUnicodeString(filePath, targetPath);

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [path=%s]", IOCTL_FORCE_DELETE_UNICODE, __WFUNCTION__.c_str(), path.c_str());
		BOOL status = DeviceIoControl(driverDevice, IOCTL_FORCE_DELETE_UNICODE, filePath, sizeof(filePath), NULL, 0, 0, NULL);

		if (status) {
			status = DeleteFileW(path.c_str());
			LOG_INFO(L"KernelInstance", L"Post-deleted file.");
		}

		return status;
	}

	BOOL KernelInstance::DeleteFileAuto(std::wstring path) noexcept {
		if (!fs::exists(path)) {
			return FALSE;
		}

		if (!fs::is_directory(path)) {
			return DeleteFileW(path.c_str());
		}
		else {
			for (const auto& entry : fs::directory_iterator(path)) {
				if (fs::is_directory(entry)) {
					DeleteFileAuto(entry.path().wstring());
				}
				if (fs::is_regular_file(entry)) {
					DeleteFileW(entry.path().wstring().c_str());
				}
			}
			LOG_INFO(L"KernelInstance", L"Post-deleted directory.");
			return RemoveDirectoryW(path.c_str());
		}
	}

	BOOL KernelInstance::_DeleteFileAuto(std::wstring path) noexcept {
		if (!fs::exists(path)) {
			return FALSE;
		}

		if (!fs::is_directory(path)) {
			return _DeleteFile(path);
		}
		else {
			for (const auto& entry : fs::directory_iterator(path)) {
				if (fs::is_directory(entry)) {
					_DeleteFileAuto(entry.path().wstring());
				}
				if (fs::is_regular_file(entry)) {
					_DeleteFile(entry.path().wstring());
				}
			}
			LOG_INFO(L"KernelInstance", L"Post-deleted directory.");
			return RemoveDirectoryW(path.c_str());
		}
	}

	BOOL KernelInstance::MurderFileAuto(std::wstring path) noexcept {
		if (!fs::exists(path)) {
			return FALSE;
		}

		if (!fs::is_directory(path)) {
			return MurderFile(path);
		}
		else {
			for (const auto& entry : fs::directory_iterator(path)) {
				if (fs::is_directory(entry)) {
					MurderFileAuto(entry.path().wstring());
				}
				if (fs::is_regular_file(entry)) {
					MurderFile(entry.path().wstring());
				}
			}
			LOG_INFO(L"KernelInstance", L"Post-deleted directory.");
			return RemoveDirectoryW(path.c_str());
		}
	}

	BOOL KernelInstance::LockFile(std::wstring path) noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		WCHAR targetPath[MAX_PATH];
		wcscpy_s(targetPath, L"\\??\\");
		wcscat_s(targetPath, path.c_str());

		UNICODE_STRING filePath[MAX_PATH];
		RtlInitUnicodeString(filePath, targetPath);

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [path=%s]", IOCTL_LOCK_FILE_UNICODE, __WFUNCTION__.c_str(), path.c_str());
		return DeviceIoControl(driverDevice, IOCTL_LOCK_FILE_UNICODE, filePath, sizeof(filePath), NULL, 0, 0, NULL);
	}

	BOOL KernelInstance::_CopyFile(std::wstring from, std::wstring to, std::wstring name) noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		struct INPUT
		{
			UNICODE_STRING FilePath[MAX_PATH];
			UNICODE_STRING FileName[MAX_PATH];
			UNICODE_STRING TargetFilePath[MAX_PATH];
		};

		WCHAR fromPath[MAX_PATH];
		wcscpy_s(fromPath, L"\\??\\");
		wcscat_s(fromPath, from.c_str());
		WCHAR toPath[MAX_PATH];
		wcscpy_s(toPath, L"\\??\\");
		wcscat_s(toPath, to.c_str());

		INPUT input = { 0 };
		RtlInitUnicodeString(input.FilePath, fromPath);
		RtlInitUnicodeString(input.FileName, name.c_str());
		RtlInitUnicodeString(input.TargetFilePath, toPath);

		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [from=%s, to=%s, name=%s]", IOCTL_NTFS_COPY_FILE, __WFUNCTION__.c_str(), from.c_str(), to.c_str(), name.c_str());
		return DeviceIoControl(driverDevice, IOCTL_NTFS_COPY_FILE, &input, sizeof(INPUT), NULL, sizeof(ULONG), 0, 0);
	}

	BOOL KernelInstance::EnableCreateProcess() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_UNPROHIBIT_CREATEPROCESS, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_UNPROHIBIT_CREATEPROCESS, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableCreateProcess() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_PROHIBIT_CREATEPROCESS, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_PROHIBIT_CREATEPROCESS, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableCreateFile() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_UNPROHIBIT_CREATEFILE, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_UNPROHIBIT_CREATEFILE, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableCreateFile() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_PROHIBIT_CREATEFILE, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_PROHIBIT_CREATEFILE, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableLoadDriver() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_UNPROHIBIT_LOADDRIVER, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_UNPROHIBIT_LOADDRIVER, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableLoadDriver() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_PROHIBIT_LOADDRIVER, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_PROHIBIT_LOADDRIVER, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableUnloadDriver() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_UNPROHIBIT_UNLOADDRIVER, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_UNPROHIBIT_UNLOADDRIVER, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableUnloadDriver() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_PROHIBIT_UNLOADDRIVER, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_PROHIBIT_UNLOADDRIVER, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableModifyRegistry() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_UNPROHIBIT_MODIFY_REGISTRY, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_UNPROHIBIT_MODIFY_REGISTRY, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableModifyRegistry() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_PROHIBIT_MODIFY_REGISTRY, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_PROHIBIT_MODIFY_REGISTRY, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::ProtectDisk() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_PROTECT_DISK, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_PROTECT_DISK, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::UnprotectDisk() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_UNPROTECT_DISK, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_UNPROTECT_DISK, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableObCallback() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_ENABLE_OBCALLBACK, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_ENABLE_OBCALLBACK, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableObCallback() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_DISABLE_OBCALLBACK, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_DISABLE_OBCALLBACK, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableDSE() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_ENABLE_DSE, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_ENABLE_DSE, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableDSE() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_DISABLE_DSE, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_DISABLE_DSE, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableCmpCallback() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_ENABLE_CMPCALLBACK, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_ENABLE_CMPCALLBACK, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableCmpCallback() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_DISABLE_CMPCALLBACK, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_DISABLE_CMPCALLBACK, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::EnableLKD() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_ENABLE_LKD, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_ENABLE_LKD, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisableLKD() noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_DISABLE_LKD, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_DISABLE_LKD, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::DisablePatchGuard(int type) noexcept {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		if (type == 0) {
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [type=%d]", IOCTL_DISABLE_PATCHGUARD_EFI, __WFUNCTION__, type);
			return DeviceIoControl(driverDevice, IOCTL_DISABLE_PATCHGUARD_EFI, NULL, 0, NULL, 0, NULL, NULL);
		}
		else if (type == 1) {
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [type=%d]", IOCTL_DISABLE_PATCHGUARD_BIOS, __WFUNCTION__, type);
			return DeviceIoControl(driverDevice, IOCTL_DISABLE_PATCHGUARD_BIOS, NULL, 0, NULL, 0, NULL, NULL);
		}
		else if (type == 2) {
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [type=%d]", IOCTL_DISABLE_PATCHGUARD_DYNAMIC, __WFUNCTION__, type);
			return DeviceIoControl(driverDevice, IOCTL_DISABLE_PATCHGUARD_DYNAMIC, NULL, 0, NULL, 0, NULL, NULL);
		}
		return FALSE;
	}

	BOOL KernelInstance::Shutdown() {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_SHUTDOWN, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_SHUTDOWN, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::Reboot() {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_REBOOT, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_REBOOT, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::RebootForce() {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;
		LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: []", IOCTL_FORCE_REBOOT, __WFUNCTION__);
		return DeviceIoControl(driverDevice, IOCTL_FORCE_REBOOT, NULL, 0, NULL, 0, NULL, NULL);
	}

	BOOL KernelInstance::BlueScreen(int color) {
		if (!GetDriverDevice() || !IsRunningAsAdmin()) return FALSE;

		if (color == -1) {
			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [color=-1]", IOCTL_BLUESCREEN, __WFUNCTION__, color);
			return DeviceIoControl(driverDevice, IOCTL_BLUESCREEN, NULL, 0, NULL, 0, NULL, NULL);
		}
		else {
			struct INPUT {
				ULONG color;
			};
			INPUT input = { color };

			LOG_WARNING(L"KernelInstance", L"Calling 0x%x from \"%s\", parameters: [color=%d]", IOCTL_CRASH_SYSTEM_SET_COLOR, __WFUNCTION__, color);
			return DeviceIoControl(driverDevice, IOCTL_CRASH_SYSTEM_SET_COLOR, &input, sizeof(input), NULL, 0, NULL, NULL);
		}
	}


	// =================================
	//				PRIVATE
	// =================================

	/*
	* 获取驱动设备位置
	*/
	BOOL KernelInstance::GetDriverDevice() noexcept {
		if (driverDevice != NULL) return TRUE;
		if (!DriverUtils::LoadKernelDriver(kernelPath.c_str(), unused)) return FALSE;

		HANDLE device = CreateFile(L"\\\\.\\ArkDrv64", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (device == INVALID_HANDLE_VALUE) return FALSE;

		driverDevice = device;
		return TRUE;
	}

	BOOL KernelInstance::GetDriverDevice2() noexcept {
		if (driverDevice2 != NULL) return TRUE;
		if (!DriverUtils::LoadKernelDriver(astralPath.c_str(), unused)) return FALSE;

		HANDLE device = CreateFile(L"\\\\.\\AstralX", GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_EXISTING, 0, NULL);

		if (device == INVALID_HANDLE_VALUE) return FALSE;

		driverDevice2 = device;
		return TRUE;
	}

	bool KernelInstance::IsRunningAsAdmin() noexcept {
		static int flag = 0;
		if (flag == 0) {
			HANDLE hToken = nullptr;
			if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
				return false;
			}

			TOKEN_ELEVATION elevation{};
			DWORD dwSize;
			bool result = GetTokenInformation(hToken, TokenElevation, &elevation, sizeof(elevation), &dwSize);
			CloseHandle(hToken);

			result = result && elevation.TokenIsElevated;

			if (result) {
				LOG_WARNING(L"KernelInstance", L"Admin privilege detected, enabling driver loading stage and related features.");
				flag = 1;
			}
			else {
				LOG_WARNING(L"KernelInstance", L"Admin privilege undetected, disabling driver loading stage and related features.");
				flag = 2;
			}

			return result;
		}
		else {
			return flag == 1;
		}
	}
}