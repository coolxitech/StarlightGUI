#pragma once

#include <windows.h>
#include <winioctl.h>
#include <sstream>

/*
* IOCTL for STARLIGHT GUI KERNEL DRIVER (SKT64)
*/
#define IOCTL_DISABLE_DSE                  CTL_CODE(FILE_DEVICE_UNKNOWN, 0x416, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ENABLE_DSE                   CTL_CODE(FILE_DEVICE_UNKNOWN, 0x417, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_ENUM_PROCESS                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x835, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_ENUM_PROCESS_THREAD_CIDTABLE CTL_CODE(FILE_DEVICE_UNKNOWN, 0x848, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_TERMINATE_PROCESS		       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x900, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FORCE_TERMINATE_PROCESS      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x903, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SUSPEND_PROCESS              CTL_CODE(FILE_DEVICE_UNKNOWN, 0x904, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_RESUME_PROCESS               CTL_CODE(FILE_DEVICE_UNKNOWN, 0x905, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_HIDE_PROCESS                 CTL_CODE(FILE_DEVICE_UNKNOWN, 0x906, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_PPL                      CTL_CODE(FILE_DEVICE_UNKNOWN, 0x908, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SET_CRITICAL_PROCESS         CTL_CODE(FILE_DEVICE_UNKNOWN, 0x910, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_TERMINATE_THREAD             CTL_CODE(FILE_DEVICE_UNKNOWN, 0x919, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SUSPEND_THREAD               CTL_CODE(FILE_DEVICE_UNKNOWN, 0x925, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_RESUME_THREAD                CTL_CODE(FILE_DEVICE_UNKNOWN, 0x926, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_FORCE_TERMINATE_THREAD       CTL_CODE(FILE_DEVICE_UNKNOWN, 0x927, METHOD_BUFFERED, FILE_ANY_ACCESS)

/*
* PPL levels
*/
#define PPL_WinSystem 0x71
#define PPL_WinTcb 0x61
#define PPL_Windows 0x51
#define PPL_Lsa 0x41
#define PPL_Antimalware 0x31
#define PPL_Codegen 0x21
#define PPL_Authenticode 0x11
#define PPL_None 0x00

typedef struct _DATA_INFO_
{
    HANDLE handledata1;
    HANDLE handledata2;
    HANDLE handledata3;
    HANDLE handledata4;
    PVOID pvoidaddressdata1;
    PVOID pvoidaddressdata2;
    PVOID pvoidaddressdata3;
    PVOID pvoidaddressdata4;
    ULONG ulongdata1;
    ULONG ulongdata2;
    ULONG ulongdata3;
    ULONG ulongdata4;
    ULONG64 ulong64data1;
    ULONG64 ulong64data2;
    ULONG64 ulong64data3;
    ULONG64 ulong64data4;
    PUCHAR puchardata1;
    char Module[MAX_PATH];
    char Module1[MAX_PATH];
    WCHAR wcstr[MAX_PATH];
    WCHAR wcstr1[MAX_PATH];
    WCHAR wcstr2[MAX_PATH];
    WCHAR wcstr3[MAX_PATH];
}DATA_INFO, * PDATA_INFO;

typedef enum _KTHREAD_STATE
{
    ThreadState_Initialized,
    ThreadState_Ready,
    ThreadState_Running,
    ThreadState_Standby,
    ThreadState_Terminated,
    ThreadState_Waiting,
    ThreadState_Transition,
    ThreadState_DeferredReady,
    ThreadState_GateWait
} KTHREAD_STATE, * PKTHREAD_STATE;