// TestDLL.cpp : Defines the exported functions for the DLL application.
//

#include <Windows.h>
#include <Shlobj.h>
#include <aclapi.h>
#include <shellapi.h>

#include "ProcessUtils.h"
#include "utils.h"

#include "ZenovaLauncher.h"

AppUtils::AppDebugger* app;

void CreateMinecraftInstance()
{
	if (!app)
	{
		std::wstring AppFullName = AppUtils::GetMinecraftPackageId();
		app = new AppUtils::AppDebugger(AppFullName);
	}
}

void LaunchMinecraft(bool forceRestart)
{
	HRESULT hresult = S_OK;

	if (!app) return;

	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		if (app->GetPackageExecutionState() != PES_UNKNOWN)
		{
			if (forceRestart)
			{
				app->TerminateAllProcesses();
				hresult = app->GetHRESULT();
				if (hresult != S_OK)
				{
					//std::cout << "Failed to restart Minecraft, HRESULT: " << hresult << std::endl;
					//system("PAUSE");
					return;
				}
			}
			else
				return;
		}

		std::wstring ModLoaderPath = Util::GetCurrentDirectory();
		if (ModLoaderPath.length() == 0)
		{
			//std::cout << "Failed to get the current directory" << std::endl;
			//system("PAUSE");
			CoUninitialize();
			return;//  E_FAIL;
		}

		ModLoaderPath += L"ZenovaModLoader.exe";
		if (!PathFileExists(ModLoaderPath.c_str()))
		{
			//std::cout << "Couldn't find ZenovaModLoader.exe" << std::endl;
			//system("PAUSE");
			CoUninitialize();
			return;//  E_FAIL;
		}
		
		app->EnableDebugging(ModLoaderPath);
		hresult = app->GetHRESULT();
		if (hresult != S_OK)
		{
			//std::cout << "Could not enable debugging, HRESULT: " << hresult << std::endl;
			//system("PAUSE");
			CoUninitialize();
			return;//  hresult;
		}

		DWORD dwProcessId = 0;
		std::wstring ApplicationId = AppUtils::GetMinecraftApplicationId();
		if (ApplicationId.length() == 0) return;// E_FAIL;
		hresult = AppUtils::LaunchApplication(ApplicationId.c_str(), &dwProcessId);
		if (hresult != S_OK)
		{
			//std::cout << "Failed to launch Minecraft, HRESULT: " << hresult << std::endl;
			//system("PAUSE");
			CoUninitialize();
			return;
		}
		else
		{
			//std::cout << "Sucessfully launched Minecraft with mods\n";
		}

		app->DisableDebugging();
		hresult = app->GetHRESULT();
		if (hresult != S_OK)
		{
			//std::cout << "Could not disable debugging, HRESULT: " << hresult << std::endl;
			//system("PAUSE");
			CoUninitialize();
			return;//  hresult;
		}

		//system("PAUSE");
		CoUninitialize();
	}

	return;//  S_OK;
}

int GetMinecraftExecutionState()
{
	// 5 is a custom enum specifying there was an error
	PACKAGE_EXECUTION_STATE executionState = (PACKAGE_EXECUTION_STATE)5;

	if (!app) return executionState;

	if (SUCCEEDED(CoInitializeEx(NULL, COINIT_APARTMENTTHREADED)))
	{
		executionState = app->GetPackageExecutionState();

		CoUninitialize();
	}

	return executionState;//  S_OK;
}

// Opens the Minecraft AppData folder in Windows Explorer
void OpenMinecraftFolder()
{
	std::wstring minecraftFolder = Util::GetMinecraftAppDataPath();
	ShellExecute(NULL, L"open", minecraftFolder.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

// Opens the AppData\Zenova\Mods folder in Windows Explorer
void OpenModsFolder()
{
	std::wstring modsFolder = Util::GetCommonFolder(FOLDERID_RoamingAppData) + L"\\Zenova\\Mods";
	ShellExecute(NULL, L"open", modsFolder.c_str(), NULL, NULL, SW_SHOWDEFAULT);
}

Callback Handler = 0;

void StateChangeCallback(PACKAGE_EXECUTION_STATE state)
{
	Handler(state);
}

void SetStateChangeCallback(Callback handler)
{
	Handler = handler;

	if (!app) return;

	app->setStateChangeCallback(&StateChangeCallback);
}

void UnregisterStateChanges()
{
	if (!app) return;

	app->stopListening();
}
