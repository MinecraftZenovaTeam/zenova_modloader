#include <Windows.h>
#include <Tlhelp32.h>
#include <iostream>
#include <ShObjIdl.h>
#include <atlbase.h>

#define MINECRAFT_APP_NAME L"Microsoft.MinecraftUWP_8wekyb3d8bbwe!App"
#define STATUS_SUCCESS 0
#define ThreadQuerySetWin32StartAddress 9

int argCount;
std::string DLLPath;

// Windows Internal Functions
typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);
typedef NTSTATUS(NTAPI *NTQUERYINFOTHREAD)(HANDLE, LONG, PVOID, ULONG, PULONG);

void SuspendProcess(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	NtSuspendProcess pfnNtSuspendProcess = (NtSuspendProcess)GetProcAddress(
		GetModuleHandle(L"ntdll"), "NtSuspendProcess");

	pfnNtSuspendProcess(processHandle);
	CloseHandle(processHandle);
}

void ResumeProcess(DWORD processId)
{
	HANDLE processHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);

	NtResumeProcess pfnNtResumeProcess = (NtResumeProcess)GetProcAddress(
		GetModuleHandle(L"ntdll"), "NtResumeProcess");

	pfnNtResumeProcess(processHandle);
	CloseHandle(processHandle);
}

DWORD FindProcessId(const std::wstring& processName)
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

// Didn't want to reinvent the wheel
// From this "tutorial": http://www.mpgh.net/forum/showthread.php?t=332194
BOOL injectDLL(DWORD procID, std::string DLLPath)
{
	// Find the address of the LoadLibrary api, luckily for us, it is loaded in the same address for every process
	HMODULE hLocKernel32 = GetModuleHandle(L"Kernel32");
	FARPROC hLocLoadLibrary = GetProcAddress(hLocKernel32, "LoadLibraryA");

	// Adjust token privileges to open system processes
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, 0, &tkp, sizeof(tkp), NULL, NULL);
	}

	// Open the process with all access
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, procID);

	// Allocate memory to hold the path to the Dll File in the process's memory
	DLLPath += '\0';
	LPVOID hRemoteMem = VirtualAllocEx(hProc, NULL, DLLPath.size(), MEM_COMMIT, PAGE_READWRITE);

	// Write the path to the Dll File in the location just created
	SIZE_T numBytesWritten;
	WriteProcessMemory(hProc, hRemoteMem, DLLPath.c_str(), DLLPath.size(), &numBytesWritten);

	// Create a remote thread that starts begins at the LoadLibrary function and is passed are memory pointer
	HANDLE hRemoteThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)hLocLoadLibrary, hRemoteMem, 0, NULL);

	// Wait for the thread to finish
	BOOL res = TRUE;
	//if (hRemoteThread)
	//	res = (BOOL)WaitForSingleObject(hRemoteThread, 10000) != WAIT_TIMEOUT;

	// Free the memory created on the other process
	//VirtualFreeEx(hProc, hRemoteMem, dll.size(), MEM_RELEASE);

	// Release the handle to the other process
	CloseHandle(hProc);

	return res;
}

// Returns the name of the module owning the thread
// Stores the full module path in wstrModulePath
// Stores the base address of the module in pModuleStartAddr
// Stores the base address of the thread in pThreadStartAddr
std::wstring GetThreadModuleName(DWORD dwProcId, HANDLE hThread, std::wstring& wstrModulePath, PDWORD_PTR pModuleStartAddr, PDWORD_PTR pThreadStartAddr)
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

// Suspends all threads in a given process owned by a given module
// Returns if it's successful
BOOL SuspendModuleThreads(const DWORD dwProcessId, const std::wstring ModuleName)
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

	if (Thread32First(hSnapshot, &threadEntry32))
	{
		// Check first thread; isn't necessary because the first thread is usually the main thread
		if (threadEntry32.th32OwnerProcessID == dwProcessId)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry32.th32ThreadID);

			wstrModuleName = GetThreadModuleName(dwProcessId, hThread, wstrModuleName, &dwptrModuleBaseAddr, &dwptrThreadBaseAddr);

			if (wstrModuleName == ModuleName)
			{
				SuspendThread(hThread);
			}
			CloseHandle(hThread);
		}

		// Go through all other threads
		while (Thread32Next(hSnapshot, &threadEntry32))
		{
			if (threadEntry32.th32OwnerProcessID == dwProcessId)
			{
				hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry32.th32ThreadID);

				wstrModuleName = GetThreadModuleName(dwProcessId, hThread, wstrModuleName, &dwptrModuleBaseAddr, &dwptrThreadBaseAddr);

				if (wstrModuleName == ModuleName)
				{
					SuspendThread(hThread);
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

// Resumes all threads in a given process owned by a given module
// Returns if it's successful
BOOL ResumeModuleThreads(const DWORD dwProcessId, const std::wstring ModuleName)
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

	if (Thread32First(hSnapshot, &threadEntry32))
	{
		// Check first thread; isn't necessary because the first thread is usually the main thread
		if (threadEntry32.th32OwnerProcessID == dwProcessId)
		{
			hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry32.th32ThreadID);

			wstrModuleName = GetThreadModuleName(dwProcessId, hThread, wstrModuleName, &dwptrModuleBaseAddr, &dwptrThreadBaseAddr);

			if (wstrModuleName == ModuleName)
			{
				ResumeThread(hThread);
			}
			CloseHandle(hThread);
		}

		// Go through all other threads
		while (Thread32Next(hSnapshot, &threadEntry32))
		{
			if (threadEntry32.th32OwnerProcessID == dwProcessId)
			{
				hThread = OpenThread(THREAD_ALL_ACCESS, FALSE, threadEntry32.th32ThreadID);

				wstrModuleName = GetThreadModuleName(dwProcessId, hThread, wstrModuleName, &dwptrModuleBaseAddr, &dwptrThreadBaseAddr);

				if (wstrModuleName == ModuleName)
				{
					ResumeThread(hThread);
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

// Will constantly keep checking if Minecraft is launched, and inject a DLL when the game is finally launched
DWORD WINAPI SearchThread(LPVOID lpParameter)
{
	DWORD pid = 0;
	UINT32 counter = 0;

	// Loop until the game is found; stop searching after some set amount of tries
	while (pid == 0 && counter < 100000)
	{
		pid = FindProcessId(L"Minecraft.Win10.DX11.exe");
		counter++;
	}

	if (pid == 0)
		return E_ABORT;

	if (!SuspendModuleThreads(pid, L"Minecraft.Win10.DX11.exe"))
		return E_ABORT;
	else
		printf("Launched Minecraft.Win10.DX11.exe (%d) and paused execution\n", pid);

	if (argCount == 1)
		system("PAUSE");
	else if (argCount == 2)
	{
		if (injectDLL(pid, DLLPath))
			printf("Injected %s\n", DLLPath.substr(DLLPath.rfind('\\') + 1, DLLPath.length()).c_str());
		else
			printf("Failed to inject %s\n", DLLPath.substr(DLLPath.rfind('\\') + 1, DLLPath.length()).c_str());
	}

	if (!ResumeModuleThreads(pid, L"Minecraft.Win10.DX11.exe"))
		return E_ABORT;
	else
		printf("Resumed Minecraft.Win10.DX11.exe (%d)\n", pid);

	return S_OK;
}

HRESULT LaunchApp(const std::wstring& strAppUserModelId, PDWORD pdwProcessId)
{
	CComPtr<IApplicationActivationManager> spAppActivationManager;
	HRESULT hrResult = E_INVALIDARG;

	if (!strAppUserModelId.empty())
	{
		// Instantiate IApplicationActivationManager
		hrResult = CoCreateInstance(CLSID_ApplicationActivationManager,
			NULL,
			CLSCTX_LOCAL_SERVER,
			IID_IApplicationActivationManager,
			(LPVOID*)&spAppActivationManager);

		if (SUCCEEDED(hrResult))
		{
			// This call ensures that the app is launched as the foreground window
			hrResult = CoAllowSetForegroundWindow(spAppActivationManager, NULL);

			// Launch the app
			if (SUCCEEDED(hrResult))
			{
				// Start the thread to search for the process
				DWORD dwSearchThreadID;
				HANDLE hSearchThread = CreateThread(0, 0, SearchThread, NULL, NULL, &dwSearchThreadID);

				// Set thread priority to the highest priority to reduce the amount of context switching
				SetThreadPriority(hSearchThread, THREAD_PRIORITY_TIME_CRITICAL);

				hrResult = spAppActivationManager->ActivateApplication(strAppUserModelId.c_str(), NULL, AO_NONE, pdwProcessId);

				// This line is only reached once the app has resumed and the ApplicationManager recieves a reply from it
				TerminateThread(hSearchThread, S_OK);
			}
		}
	}

	return hrResult;
}

wchar_t* toWCHAR(const char* text)
{
	size_t size = strlen(text) + 1;
	size_t output;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(&output, wa, size, text, size);
	return wa;
}

int main(int argc, char* argv[])
{
	// Set process priority to the highest priority to reduce the amount of context switching
	SetPriorityClass(GetCurrentProcess(), REALTIME_PRIORITY_CLASS);

	argCount = argc;

	if (argc < 1 || argc > 2)
	{
		std::cout << "Too many arguments" << std::endl;
		return E_INVALIDARG;
	}
	else if (argc == 2 && !PathFileExists(toWCHAR(argv[1])))
	{
		printf("No file found at: %s\n", argv[1]);
		return E_INVALIDARG;
	}

	if (argc == 2)
	{
		std::string DLL(argv[1]);
		DLLPath = DLL;
	}

	HRESULT hrResult = S_OK;
	DWORD dwProcessId;
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		dwProcessId = 0;
		hrResult = LaunchApp(MINECRAFT_APP_NAME, &dwProcessId);

		CoUninitialize();
	}

	// Something went wrong and the app failed to launch
	if (dwProcessId == 0)
	{
		return E_FAIL;
	}

	return hrResult;
}
