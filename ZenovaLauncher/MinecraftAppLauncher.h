#ifndef __APPLAUNCHERMANAGER_H_
#define __APPLAUNCHERMANAGER_H_

#include <iostream>

namespace MinecraftAppLauncher
{
	/* Launches Minecraft and invokes mod injection */
	HRESULT LaunchApplicationAndInjectMods(PDWORD processId_ptr, BOOL _doInject);
	/* Gets the running processId of Minecraft */
	DWORD GetProcessId(const std::wstring& processName);
	/* Injects mods into Minecraft's main process */
	DWORD WINAPI InjectMods_Threaded(LPVOID lpParameter);
};

#endif // __APPLAUNCHERMANAGER_H_
