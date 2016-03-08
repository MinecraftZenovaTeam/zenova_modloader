#include <Windows.h>
#include <Tlhelp32.h>

#include "ThreadWorker.h"

#define STATUS_SUCCESS 0
#define ThreadQuerySetWin32StartAddress 9

/* typedef for Windows function type */
typedef NTSTATUS(NTAPI* NTQUERYINFOTHREAD)(HANDLE, LONG, PVOID, ULONG, PULONG);


/* Returns the name of the module that owns the given thread
*	Stores the full module path in wstrModulePath
*	Stores the base address of the module in pModuleStartAddr
*	Stores the base address of the thread in pThreadStartAddr
*/
std::wstring ThreadWorker::GetThreadOwnerModule(DWORD dwProcId, HANDLE hThread, std::wstring& wstrModulePath, PDWORD_PTR pModuleStartAddr, PDWORD_PTR pThreadStartAddr)
{
	NTSTATUS ntStatus;
	HANDLE hCurrentProcess, hNewThreadHandle, hSnapshot;
	DWORD_PTR dwptrThreadStartAddr = 0;
	NTQUERYINFOTHREAD NtQueryInformationThread;
	MODULEENTRY32 moduleEntry32;

	if ((NtQueryInformationThread = (NTQUERYINFOTHREAD)GetProcAddress(GetModuleHandle(L"ntdll.dll"), "NtQueryInformationThread")))
	{
		hCurrentProcess = GetCurrentProcess();
		if (DuplicateHandle(hCurrentProcess, hThread, hCurrentProcess, &hNewThreadHandle, THREAD_QUERY_INFORMATION, FALSE, 0))
		{
			ntStatus = NtQueryInformationThread(hNewThreadHandle, ThreadQuerySetWin32StartAddress, &dwptrThreadStartAddr, sizeof(DWORD_PTR), NULL);
			*pThreadStartAddr = dwptrThreadStartAddr;
			CloseHandle(hNewThreadHandle);

			if (ntStatus != STATUS_SUCCESS)
			{
				return L"";
			}
		}

	}

	hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPALL, dwProcId);

	moduleEntry32.dwSize = sizeof(MODULEENTRY32);
	moduleEntry32.th32ModuleID = 1;

	if (Module32First(hSnapshot, &moduleEntry32) && dwptrThreadStartAddr)
	{
		if (dwptrThreadStartAddr >= (DWORD_PTR)moduleEntry32.modBaseAddr && dwptrThreadStartAddr <= ((DWORD_PTR)moduleEntry32.modBaseAddr + moduleEntry32.modBaseSize))
		{
			wstrModulePath = std::wstring(moduleEntry32.szExePath);
		}
		else
		{
			while (Module32Next(hSnapshot, &moduleEntry32))
			{
				if (dwptrThreadStartAddr >= (DWORD_PTR)moduleEntry32.modBaseAddr && dwptrThreadStartAddr <= ((DWORD_PTR)moduleEntry32.modBaseAddr + moduleEntry32.modBaseSize))
				{
					wstrModulePath = std::wstring(moduleEntry32.szExePath);
					break;
				}
			}
		}
	}

	if (pModuleStartAddr)
	{
		*pModuleStartAddr = (DWORD_PTR)moduleEntry32.modBaseAddr;
	}
	CloseHandle(hSnapshot);

	return wstrModulePath.substr(wstrModulePath.rfind(L"\\") + 1, wstrModulePath.length());
}

/* Either suspends or resumes all threads in the given module */
BOOL ThreadWorker::SetModuleThreadState(const DWORD dwProcessId, const std::wstring ModuleName, BOOL _threadStatus)
{
	HANDLE hSnapshot, hThread;
	THREADENTRY32 threadEntry32;
	DWORD_PTR dwptrModuleBaseAddr, dwptrThreadBaseAddr;
	std::wstring wstrModuleName, wstrModulePath;

	if ((hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, dwProcessId)) == INVALID_HANDLE_VALUE)
	{
		return FALSE;
	}

	threadEntry32.dwSize = sizeof(THREADENTRY32);
	threadEntry32.cntUsage = 0;

	/* Resume all threads in the given module */
	if (Thread32First(hSnapshot, &threadEntry32))
	{
		if (threadEntry32.th32OwnerProcessID == dwProcessId)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry32.th32ThreadID);

			wstrModuleName = GetThreadOwnerModule(dwProcessId, hThread, wstrModuleName, &dwptrModuleBaseAddr, &dwptrThreadBaseAddr);

			if (wstrModuleName == ModuleName)
			{
				if (_threadStatus)
				{
					ResumeThread(hThread);
				}
				else
				{
					SuspendThread(hThread);
				}
			}
			CloseHandle(hThread);
		}

		/* Now check all other threads */
		while (Thread32Next(hSnapshot, &threadEntry32))
		{
			if (threadEntry32.th32OwnerProcessID == dwProcessId)
			{
				hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry32.th32ThreadID);

				wstrModuleName = GetThreadOwnerModule(dwProcessId, hThread, wstrModuleName, &dwptrModuleBaseAddr, &dwptrThreadBaseAddr);

				if (wstrModuleName == ModuleName)
				{
					if (_threadStatus)
					{
						ResumeThread(hThread);
					}
					else
					{
						SuspendThread(hThread);
					}
				}
				CloseHandle(hThread);
			}
		}
	}
	else
	{
		return FALSE;
	}

	CloseHandle(hSnapshot);

	return TRUE;
}
