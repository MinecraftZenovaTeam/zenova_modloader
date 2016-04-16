#include <Windows.h>
#include <ShObjIdl.h>
#include <AppxPackaging.h>
#include <atlbase.h>
#include <string>
#include <wrl.h>

#include "AppUtils.h"

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

HRESULT AppUtils::LaunchApplication(LPCWSTR packageFullName, PDWORD pdwProcessId)
{
	CComPtr<IApplicationActivationManager> spAppActivationManager;
	HRESULT result = E_INVALIDARG;

	/* Initialize IApplicationActivationManager */
	result = CoCreateInstance(CLSID_ApplicationActivationManager,
		NULL,
		CLSCTX_LOCAL_SERVER,
		IID_IApplicationActivationManager,
		(LPVOID*)&spAppActivationManager);

	if (!SUCCEEDED(result)) return result;

	/* This call ensures that the app is launched as the foreground window */
	result = CoAllowSetForegroundWindow(spAppActivationManager, NULL);

	/* Launch the app */
	if (!SUCCEEDED(result)) return result;

	result = spAppActivationManager->ActivateApplication(packageFullName, NULL, AO_NONE, pdwProcessId);

	return result;
}

AppUtils::StateChangeMonitor::StateChangeMonitor() {}

AppUtils::StateChangeMonitor::~StateChangeMonitor()
{
	if (this->packageDebugSettings) {
		this->packageDebugSettings->UnregisterForPackageStateChanges(this->registrationId);
		this->registrationId = 0;
		this->packageDebugSettings = nullptr;
	}
}

void AppUtils::StateChangeMonitor::initialize(const wchar_t *packageFullName, ATL::CComPtr<IPackageDebugSettings> packageDebugSettingsIn)
{
	this->registrationId = 0;
	this->packageDebugSettings = packageDebugSettingsIn;
	this->packageDebugSettings->RegisterForPackageStateChanges(packageFullName, this, &this->registrationId);
}

HRESULT __stdcall AppUtils::StateChangeMonitor::OnStateChanged(const wchar_t *packageFullName, PACKAGE_EXECUTION_STATE state)
{
	//showStateMessageBox(state);
	if (this->callbackPtr)
	{
		this->callbackPtr(state);
	}
	return S_OK;
}

PACKAGE_EXECUTION_STATE AppUtils::StateChangeMonitor::getState() {
	return currentState;
}

void AppUtils::StateChangeMonitor::setCallback(void* CallbackPtr) {
	if (CallbackPtr)
		callbackPtr = (callbackfn)CallbackPtr;
}

void AppUtils::StateChangeMonitor::showStateMessageBox(PACKAGE_EXECUTION_STATE state) {
	std::wstring str;
	switch (state) {
	case PES_RUNNING:
		str = L"Running"; break;
	case PES_SUSPENDING:
		str = L"Suspending"; break;
	case PES_SUSPENDED:
		str = L"Suspended"; break;
	case PES_TERMINATED:
		str = L"Terminated"; break;
	case PES_UNKNOWN:
		str = L"Unknown"; break;
	default:
		str = L"Uknown Execution State: " + std::to_wstring((int)state); break;
	}
	MessageBox(NULL, str.c_str(), L"Execution State", MB_OK);
	currentState = state;
}

AppUtils::AppDebugger::~AppDebugger() {}

AppUtils::AppDebugger::AppDebugger(const std::wstring& packageFullName)
{
	PackageFullName = packageFullName;
	hResult = S_OK;
	hResult = debugSettings.CoCreateInstance(CLSID_PackageDebugSettings, NULL, CLSCTX_ALL);
	stateChangeMonitor = Microsoft::WRL::Make<AppUtils::StateChangeMonitor>();
	stateChangeMonitor->initialize(packageFullName.c_str(), debugSettings);
}

void AppUtils::AppDebugger::stopListening() {
	if (stateChangeMonitor)
	{
		stateChangeMonitor.Reset();
		stateChangeMonitor = nullptr;
	}
}

HRESULT AppUtils::AppDebugger::GetHRESULT()
{
	return hResult;
}

void AppUtils::AppDebugger::setStateChangeCallback(void* callbackPtr)
{
	stateChangeMonitor->setCallback(callbackPtr);
}

void AppUtils::AppDebugger::DisableDebugging()
{
	hResult = E_FAIL;
	if (debugSettings == NULL) return;

	hResult = debugSettings->DisableDebugging(PackageFullName.c_str());
}

void AppUtils::AppDebugger::EnableDebugging(std::wstring commandLineParameters)
{
	hResult = E_FAIL;
	if (debugSettings == NULL) return;

	hResult = debugSettings->EnableDebugging(PackageFullName.c_str(), commandLineParameters.c_str(), NULL);
}

void AppUtils::AppDebugger::Suspend()
{
	hResult = E_FAIL;
	if (debugSettings == NULL) return;

	hResult = debugSettings->Suspend(PackageFullName.c_str());
}

void AppUtils::AppDebugger::Resume()
{
	hResult = E_FAIL;
	if (debugSettings == NULL) return;

	hResult = debugSettings->Resume(PackageFullName.c_str());
}

void AppUtils::AppDebugger::TerminateAllProcesses()
{
	hResult = E_FAIL;
	if (debugSettings == NULL) return;
	
	hResult = debugSettings->TerminateAllProcesses(PackageFullName.c_str());
}

PACKAGE_EXECUTION_STATE AppUtils::AppDebugger::GetPackageExecutionState()
{
	hResult = E_FAIL;
	if (debugSettings == NULL) return PES_UNKNOWN;

	PACKAGE_EXECUTION_STATE packageState = PES_UNKNOWN;
	hResult = debugSettings->GetPackageExecutionState(PackageFullName.c_str(), &packageState);

	return packageState;
}