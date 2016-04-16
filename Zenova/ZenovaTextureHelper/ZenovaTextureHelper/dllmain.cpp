#include <Windows.h>
#include <atlbase.h>
#include <Shlobj.h>
#include <string>
#include "MinHook.h"

std::wstring MINECRAFT_LOCAL_PATH;

// Hooks a function at a given address given the hook function and trampoline function
BOOL setHook(LPVOID* origAddress, LPVOID* hookFunction, LPVOID* trampFunction)
{
	if (MH_CreateHook(origAddress, hookFunction, reinterpret_cast<LPVOID*>(trampFunction)) != MH_OK)
		return FALSE;

	if (MH_EnableHook(origAddress) != MH_OK)
		return FALSE;

	return TRUE;
}

// Attaches a hook on a function given the name of the owning module and the name of the function
BOOL attach(LPWSTR wstrModule, LPCSTR strFunction, LPVOID* hook, LPVOID* original)
{
	HMODULE hModule = GetModuleHandle(wstrModule);
	if (hModule == NULL)
		return FALSE;

	FARPROC hFunction = GetProcAddress(hModule, strFunction);
	if (hFunction == NULL)
		return FALSE;

	return setHook((LPVOID*)hFunction, hook, original);
}

// Hook setup for CreateFileW
typedef HANDLE(WINAPI *PfnCreateFileW)(LPCWSTR lpFilename, DWORD dwAccess, DWORD dwSharing, LPSECURITY_ATTRIBUTES saAttributes, DWORD dwCreation, DWORD dwAttributes, HANDLE hTemplate);
PfnCreateFileW pfnCreateFileW = NULL;

HANDLE WINAPI HfnCreateFileW(LPCWSTR lpFilename, DWORD dwAccess, DWORD dwSharing, LPSECURITY_ATTRIBUTES saAttributes, DWORD dwCreation, DWORD dwAttributes, HANDLE hTemplate)
{
	std::wstring filePath(lpFilename);
	
	// Check if it's accessing resources; this method should work in all future updates
	if (filePath.find(L"C:\\Program Files\\WindowsApps\\Microsoft.Minecraft") != filePath.npos && filePath.find(L"/data/") != filePath.npos)
	{
		
		std::wstring newPath(MINECRAFT_LOCAL_PATH);
		newPath = newPath + filePath.substr(filePath.find(L"/data/") + 5, filePath.size());

		// Check if a file exists at that path and reroute access to that file
		if (PathFileExists(newPath.c_str()))
			return pfnCreateFileW(newPath.c_str(), dwAccess, dwSharing, saAttributes, dwCreation, dwAttributes, hTemplate);
	}

	return pfnCreateFileW(lpFilename, dwAccess, dwSharing, saAttributes, dwCreation, dwAttributes, hTemplate);
}

BOOL Initialize()
{
	if (MH_Initialize() != MH_OK)
		return FALSE;

	// Get the path to the games's AppData folder
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
	{
		// Get the path to the textures folder
		std::wstring appData(szPath);
		appData = appData.substr(0, appData.rfind(L"AC"));
		appData += L"LocalState\\games\\com.mojang\\assets\\";

		MINECRAFT_LOCAL_PATH = appData;
	}
	else
		return FALSE;

	// Attach a hook on CreateProcessW and return the status of the hook
	BOOL hook = TRUE;
	hook &= attach(L"KernelBase.dll", "CreateFileW", (LPVOID*)&HfnCreateFileW, (LPVOID*)&pfnCreateFileW);

	return hook;
}

BOOL Uninitialize()
{
	if (MH_Uninitialize() != MH_OK)
		return FALSE;

	return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		if (!Initialize())
		{
			// Something went wrong, so force the DLL to detach and clean-up
			return FALSE;
		}
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		Uninitialize();
		break;
	}
	return TRUE;
}