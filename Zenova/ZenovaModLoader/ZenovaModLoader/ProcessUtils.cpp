#include <Windows.h>
#include <ShObjIdl.h>
#include <Shlobj.h>
#include <Tlhelp32.h>
#include <string>

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

/* Gets the running processId of an application given its process name */
DWORD ProcessUtils::GetProcessId(const std::wstring& processName)
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
	if (processesSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(processesSnapshot, &processInfo);
	if (!processName.compare(processInfo.szExeFile))
	{
		CloseHandle(processesSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(processesSnapshot, &processInfo))
	{
		if (!processName.compare(processInfo.szExeFile))
		{
			CloseHandle(processesSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(processesSnapshot);
	return 0;
}