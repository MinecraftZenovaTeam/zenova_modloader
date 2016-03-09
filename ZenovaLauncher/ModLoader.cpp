#include <Windows.h>

#include "utils.h"
#include "ModLoader.h"

BOOL ModLoader::InjectDLL(DWORD dwProcessId, std::string dllPath)
{
	/* Find the address of the LoadLibrary API; Luckily it's loaded in the same address for every process */
	HMODULE hLocKernel32 = GetModuleHandle(L"Kernel32");
	FARPROC hLocLoadLibrary = GetProcAddress(hLocKernel32, "LoadLibraryA");

	/* Adjust token privileges to allow opening system processes */
	HANDLE hToken;
	TOKEN_PRIVILEGES tkp;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
	{
		LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &tkp.Privileges[0].Luid);
		tkp.PrivilegeCount = 1;
		tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
		AdjustTokenPrivileges(hToken, 0, &tkp, sizeof(tkp), NULL, NULL);
	}

	/* Open the process with all access */
	HANDLE hProc = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwProcessId);

	/* Allocate memory to hold the path to the DLL File in the process's memory */
	dllPath += '\0';
	LPVOID hRemoteMem = VirtualAllocEx(hProc, NULL, dllPath.size(), MEM_COMMIT, PAGE_READWRITE);

	/* Write the path to the DLL File in the memory just allocated */
	SIZE_T numBytesWritten;
	WriteProcessMemory(hProc, hRemoteMem, dllPath.c_str(), dllPath.size(), &numBytesWritten);

	/* Create a remote thread that invokes LoadLibrary for our DLL */
	HANDLE hRemoteThread = CreateRemoteThread(hProc, NULL, 0, (LPTHREAD_START_ROUTINE)hLocLoadLibrary, hRemoteMem, 0, NULL);

	CloseHandle(hProc);

	return TRUE;
}

HRESULT ModLoader::InjectMods(DWORD dwProcessId)
{
	InjectDLL(dwProcessId, Util::GetMinecraftAppDataPath() + "mods\\texturepack.dll");

	return S_OK;
}
