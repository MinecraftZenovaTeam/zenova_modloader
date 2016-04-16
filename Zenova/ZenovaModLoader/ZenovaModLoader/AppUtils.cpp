#include <Windows.h>
#include <ShObjIdl.h>

#include "AppUtils.h"

// Need to find some way to not be dependent on this string
#define MINECRAFT_DEFAULT_NAME L"Minecraft: Windows 10 Edition Beta"

std::wstring AppUtils::GetMinecraftPackageId()
{
	std::wstring packageId;

	ATL::CRegKey key, subKey;
	LPTSTR szBuffer = new TCHAR[1024], szSubBuffer = new TCHAR[1024];
	DWORD dwBufferLen = 1024, dwSubBufferLen = 1024;
	DWORD index = 0;

	LRESULT result = ERROR_SUCCESS;

	result = key.Open(HKEY_CLASSES_ROOT, L"", KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ);
	if (result != ERROR_SUCCESS) return packageId;

	for (; key.EnumKey(index, szBuffer, &dwBufferLen, NULL) == ERROR_SUCCESS; dwBufferLen = 1024, index++)
	{
		std::wstring keyName(szBuffer);
		if (keyName.substr(0, 4) != L"AppX") continue;

		result = subKey.Open(HKEY_CLASSES_ROOT, keyName.c_str(), KEY_QUERY_VALUE | KEY_READ);
		if (result != ERROR_SUCCESS) continue;

		szSubBuffer = new TCHAR[1024];
		dwSubBufferLen = 1024;
		result = subKey.QueryStringValue(L"", szSubBuffer, &dwSubBufferLen);
		if (result != ERROR_SUCCESS) continue;

		std::wstring keyValue(szSubBuffer);
		if (keyValue.find(MINECRAFT_DEFAULT_NAME) == keyValue.npos) continue;

		szSubBuffer = new TCHAR[1024];
		dwSubBufferLen = 1024;
		result = subKey.Open(HKEY_CLASSES_ROOT, (keyName + L"\\Shell\\Open").c_str(), KEY_QUERY_VALUE | KEY_READ);
		if (result != ERROR_SUCCESS) continue;

		result = subKey.QueryStringValue(L"PackageId", szSubBuffer, &dwSubBufferLen);
		if (result != ERROR_SUCCESS) continue;

		packageId = szSubBuffer;

		break;
	}

	subKey.Close();
	key.Close();

	return packageId;
}

std::wstring AppUtils::GetMinecraftApplicationId()
{
	std::wstring appId;

	ATL::CRegKey key, subKey;
	LPTSTR szBuffer = new TCHAR[1024], szSubBuffer = new TCHAR[1024];
	DWORD dwBufferLen = 1024, dwSubBufferLen = 1024;
	DWORD index = 0;

	LRESULT result = ERROR_SUCCESS;

	result = key.Open(HKEY_CLASSES_ROOT, L"", KEY_ENUMERATE_SUB_KEYS | KEY_QUERY_VALUE | KEY_READ);
	if (result != ERROR_SUCCESS) return appId;

	for (; key.EnumKey(index, szBuffer, &dwBufferLen, NULL) == ERROR_SUCCESS; dwBufferLen = 1024, index++)
	{
		std::wstring keyName(szBuffer);
		if (keyName.substr(0, 4) != L"AppX") continue;

		result = subKey.Open(HKEY_CLASSES_ROOT, keyName.c_str(), KEY_QUERY_VALUE | KEY_READ);
		if (result != ERROR_SUCCESS) continue;

		dwSubBufferLen = 1024;
		result = subKey.QueryStringValue(L"", szSubBuffer, &dwSubBufferLen);
		if (result != ERROR_SUCCESS) continue;

		std::wstring keyValue(szSubBuffer);
		if (keyValue.find(MINECRAFT_DEFAULT_NAME) == keyValue.npos) continue;

		dwSubBufferLen = 1024;
		result = subKey.Open(HKEY_CLASSES_ROOT, (keyName + L"\\Application").c_str(), KEY_QUERY_VALUE | KEY_READ);
		if (result != ERROR_SUCCESS) continue;

		result = subKey.QueryStringValue(L"AppUserModelID", szSubBuffer, &dwSubBufferLen);
		if (result != ERROR_SUCCESS) continue;

		appId = szSubBuffer;

		break;
	}

	subKey.Close();
	key.Close();

	return appId;
}

ATL::CComQIPtr<IPackageDebugSettings> AppUtils::AppDebugger::CreatePackageDebugger()
{
	ATL::CComQIPtr<IPackageDebugSettings> debugSettings;
	hResult = debugSettings.CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_ALL);
	return debugSettings;
}

AppUtils::AppDebugger::AppDebugger(const std::wstring& packageFullName)
{
	PackageFullName = packageFullName;
	hResult = S_OK;
}

HRESULT AppUtils::AppDebugger::GetHRESULT()
{
	return hResult;
}

void AppUtils::AppDebugger::DisableDebugging()
{
	hResult = E_FAIL;

	ATL::CComQIPtr<IPackageDebugSettings> debugSettings = CreatePackageDebugger();
	if (debugSettings == NULL) return;

	hResult = debugSettings->DisableDebugging(PackageFullName.c_str());
}

void AppUtils::AppDebugger::EnableDebugging(std::wstring commandLineParameters)
{
	hResult = E_FAIL;

	ATL::CComQIPtr<IPackageDebugSettings> debugSettings = CreatePackageDebugger();
	if (debugSettings == NULL) return;

	hResult = debugSettings->EnableDebugging(PackageFullName.c_str(), commandLineParameters.c_str(), NULL);
}

void AppUtils::AppDebugger::Suspend()
{
	hResult = E_FAIL;

	ATL::CComQIPtr<IPackageDebugSettings> debugSettings = CreatePackageDebugger();
	if (debugSettings == NULL) return;

	hResult = debugSettings->Suspend(PackageFullName.c_str());
}

void AppUtils::AppDebugger::Resume()
{
	hResult = E_FAIL;

	ATL::CComQIPtr<IPackageDebugSettings> debugSettings = CreatePackageDebugger();
	if (debugSettings == NULL) return;

	hResult = debugSettings->Resume(PackageFullName.c_str());
}

void AppUtils::AppDebugger::TerminateAllProcesses()
{
	hResult = E_FAIL;

	ATL::CComQIPtr<IPackageDebugSettings> debugSettings = CreatePackageDebugger();
	if (debugSettings == NULL) return;
	
	hResult = debugSettings->TerminateAllProcesses(PackageFullName.c_str());
}

PACKAGE_EXECUTION_STATE AppUtils::AppDebugger::GetPackageExecutionState()
{
	hResult = E_FAIL;

	ATL::CComQIPtr<IPackageDebugSettings> debugSettings = CreatePackageDebugger();
	if (debugSettings == NULL) return PES_UNKNOWN;

	PACKAGE_EXECUTION_STATE packageState = PES_UNKNOWN;
	hResult = debugSettings->GetPackageExecutionState(PackageFullName.c_str(), &packageState);

	return packageState;
}

void AppUtils::AppDebugger::PrintExecutionState()
{
	switch (GetPackageExecutionState())
	{
	case PES_UNKNOWN:
		std::cout << "Unknown\n";
		break;
	case PES_RUNNING:
		std::cout << "Running\n";
		break;
	case PES_SUSPENDING:
		std::cout << "Suspending\n";
		break;
	case PES_SUSPENDED:
		std::cout << "Suspended\n";
		break;
	case PES_TERMINATED:
		std::cout << "Terminated\n";
		break;
	}
}