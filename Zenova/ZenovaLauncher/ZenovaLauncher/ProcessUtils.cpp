#include <Windows.h>
#include <ShObjIdl.h>
#include <Shlobj.h>

#include "ProcessUtils.h"

void ProcessUtils::SuspendProcess(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	ProcessUtils::NtSuspendProcess pfnNtSuspendProcess = (ProcessUtils::NtSuspendProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtSuspendProcess");

	pfnNtSuspendProcess(processHandle);
	CloseHandle(processHandle);
}

void ProcessUtils::ResumeProcess(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	ProcessUtils::NtResumeProcess pfnNtResumeProcess = (ProcessUtils::NtResumeProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtResumeProcess");

	pfnNtResumeProcess(processHandle);
	CloseHandle(processHandle);
}

void ProcessUtils::TerminateProcess(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	ProcessUtils::NtTerminateProcess pfnNtTerminateProcess = (ProcessUtils::NtTerminateProcess)GetProcAddress(GetModuleHandle(L"ntdll"), "NtTerminateProcess");

	pfnNtTerminateProcess(processHandle);
	CloseHandle(processHandle);
}
