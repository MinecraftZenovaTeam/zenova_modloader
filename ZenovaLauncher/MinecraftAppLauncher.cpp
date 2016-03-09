#include <Windows.h>
#include <Tlhelp32.h>
#include <ShObjIdl.h>
#include <atlbase.h>

#include "ThreadWorker.h"
#include "ModLoader.h"

#include "MinecraftAppLauncher.h"

#define MINECRAFT_APP_NAME L"Microsoft.MinecraftUWP_8wekyb3d8bbwe!App"
#define MINECRAFT_MODULE_NAME L"Minecraft.Win10.DX11.exe"

/* Launches Minecraft and invokes mod injection */
HRESULT MinecraftAppLauncher::LaunchApplicationAndInjectMods(PDWORD processId_ptr, BOOL _doInject)
{
	CComPtr<IApplicationActivationManager> spAppActivationManager;
	HRESULT result = E_INVALIDARG;

	/* Initialize IApplicationActivationManager */
	result = CoCreateInstance(CLSID_ApplicationActivationManager,
		NULL,
		CLSCTX_LOCAL_SERVER,
		IID_IApplicationActivationManager,
		(LPVOID*)&spAppActivationManager);

	if (SUCCEEDED(result))
	{
		/* This call ensures that the app is launched as the foreground window */
		result = CoAllowSetForegroundWindow(spAppActivationManager, NULL);

		/* Launch the app */
		if (SUCCEEDED(result))
		{
			DWORD hInjectModsThreadId;
			HANDLE hInjectModsThread;

			/* Inject mods if activated */
			if (_doInject)
			{
				/* Begin thread to find Minecraft and inject mods */
				hInjectModsThread = CreateThread(0, 0, InjectMods_Threaded, NULL, NULL, &hInjectModsThreadId);

				/* Set thread priority to the highest priority to reduce the amount of context switching */
				SetThreadPriority(hInjectModsThread, THREAD_PRIORITY_TIME_CRITICAL);

				/* Initialize the application with mods */
				result = spAppActivationManager->ActivateApplication(MINECRAFT_APP_NAME, NULL, AO_NONE, processId_ptr);

				/* Terminate the search thread */
				TerminateThread(hInjectModsThread, S_OK);
			}
			else
			{
				/* Initialize the application without mods */
				result = spAppActivationManager->ActivateApplication(MINECRAFT_APP_NAME, NULL, AO_NONE, processId_ptr);
			}
		}
	}

	return result;
}

/* Gets the running processId of Minecraft */
DWORD MinecraftAppLauncher::GetProcessId(const std::wstring& processName)
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

/* Injects mods into Minecraft's main process */
DWORD WINAPI MinecraftAppLauncher::InjectMods_Threaded(LPVOID lpParameter)
{
	DWORD _processId = 0;
	UINT32 counter = 0;

	/* Search for the main Minecraft module and inject user installed mods */
	while (_processId == 0 && counter++ < 1000)
	{
		_processId = GetProcessId(MINECRAFT_MODULE_NAME);
	}

	if (_processId == 0)
	{
		std::cout << "Unable to find main Minecraft module (Minecraft.Win10.DX11.exe)\n";

		return E_ABORT;
	}

	std::cout << "Suspending Minecraft threads...\n";
	if (!ThreadWorker::SetModuleThreadState(_processId, MINECRAFT_MODULE_NAME, ThreadWorker::ThreadState::THREAD_SUSPENDED))
	{
		return E_ABORT;
	}

	std::cout << "Launched Minecraft.Win10.DX11.exe (" << _processId << ") and paused execution\n";

	if (SUCCEEDED(ModLoader::InjectMods(_processId)))
	{
		std::cout << "Successfully injected mods into Minecraft.Win10.DX11.exe (" << _processId << ")\n";
	}
	else
	{
		std::cout << "One or more errors with injecting mods. Aborting.\n";
	}

	std::cout << "Resuming Minecraft threads...\n";
	if (!ThreadWorker::SetModuleThreadState(_processId, MINECRAFT_MODULE_NAME, ThreadWorker::ThreadState::THREAD_RUNNING))
	{
		return E_ABORT;
	}

	std::cout << "Resumed Minecraft.Win10.DX11.exe (" << _processId << ")\n";
	system("PAUSE");

	return S_OK;
}
