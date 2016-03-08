#ifndef __MODLOADER_H_
#define __MODLOADER_H_

#include <iostream>

namespace ModLoader
{
	/* Inject a DLL into Minecraft */
	BOOL InjectDLL(DWORD dwProcessId, std::string dllPath);
	/* Loads mods from AppData and injects them */
	HRESULT InjectMods(DWORD dwProcessId);
};

#endif // __MODLOADER_H_
