#include <Windows.h>
#include <ShObjIdl.h>

#include "AppUtils.h"
#include "ModLoader.h"
#include "ProcessUtils.h"

#define IMPORT extern __declspec(dllimport)

IMPORT int __argc;
IMPORT char** __argv;
//IMPORT wchar_t** __wargv;

// Turning this into a normal Windows program hides the GUI
int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd)
{
	DWORD dwProcessId = 0;

	for (int i = 1; i < __argc; i += 2)
	{
		//printf("%s, %s\n", __argv[i], __argv[i+1]);
		std::string arg(__argv[i]);
		if (arg == "-p")
		{
			dwProcessId = atoi(__argv[i + 1]);
		}
	}
	
	if (dwProcessId != 0 && SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{

		std::wstring AppFullName = AppUtils::GetMinecraftPackageId();
		AppUtils::AppDebugger app(AppFullName);

		if (app.GetPackageExecutionState() == PES_UNKNOWN)
		{
			CoUninitialize();
			return E_FAIL;
		}

		// Assume the game is suspended and inject mods
		ModLoader::InjectMods(dwProcessId);

		// Resume the game
		ProcessUtils::ResumeProcess(dwProcessId);

		CoUninitialize();
	}

    return S_OK;
}