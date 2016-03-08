#include <Windows.h>

#include "MinecraftAppLauncher.h"


int main(int argc, char* argv[])
{
	HRESULT hrResult = S_OK;
	DWORD dwProcessId = 0;
	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		MinecraftAppLauncher::LaunchApplicationAndInjectMods(&dwProcessId, !(strcmp(argv[0], "-disableMods") == 0));

		CoUninitialize();
	}

	if (dwProcessId == 0)
	{
		hrResult = E_FAIL;
	}

	return hrResult;
}
