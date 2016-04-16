#ifndef __MODLOADER_H_
#define __MODLOADER_H_

#include <iostream>

namespace ModLoader
{
	/* Sets the group policy of a given file to "ALL APPLICATION PACKAGES" */
	DWORD AdjustGroupPolicy(std::wstring wstrFilePath);

	/* Inject a DLL into Minecraft */
	BOOL InjectDLL(DWORD dwProcessId, std::wstring dllPath);

	/* Loads mods from AppData and injects them */
	HRESULT InjectMods(DWORD dwProcessId);
};

#endif // __MODLOADER_H_
