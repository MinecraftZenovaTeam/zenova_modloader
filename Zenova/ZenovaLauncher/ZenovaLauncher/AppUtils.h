#ifndef __APPUTILS_H_
#define __APPUTILS_H_

/*
 * Features basic tools for dealing with Win10 apps and a wrapper for IPackageDebugSettings
 *
 * Word of note:
 * Some functions/classes take the applicationId as a parameter, while others take the packageId.
 *
 * The ApplicationId usually looks like:
 * Publisher.AppName[UWP]_PublisherId!App
 * 
 * While the PackageId usually looks like:
 * Publisher.AppName[UWP]_VersionMajor.VersionMinor.PatchMajor.PatchMinor_Chipset__PublisherId
 * 
 * If a parameter of a function is asking for either the PackageId or PackageFullName, use GetMinecraftPackageId
 *  Otherwise, if the function is asking for the ApplicationId, use GetMinecraftApplicationId.
 */

#include <Windows.h>
#include <iostream>
#include <string>
#include <ShObjIdl.h>
#include <atlbase.h>
#include <wrl.h>

using namespace Microsoft;

namespace AppUtils
{
	// String found in HKEY_CLASSES_ROOT\AppX[MinecraftPackage]
	// Need to find some way to not be dependent on this string
	#define MINECRAFT_DEFAULT_NAME L"Minecraft: Windows 10 Edition Beta"

	/* 
	 * Returns the PackageId found in the registry location:
	 * HKEY_CLASSES_ROOT\AppX[MinecraftPackage]\Shell\Open\PackageId
	 * 
	 * May look something like this:
	 * Microsoft.MinecraftUWP_0.141.0.0_x64__8wekyb3d8bbwe
	 */
	std::wstring GetMinecraftPackageId();

	/*
	* Returns the ApplicationId/AppUserModelID found in the registry location:
	* HKEY_CLASSES_ROOT\AppX[MinecraftPackage]\Application\AppUserModelID
	*
	* May look something like this:
	* Microsoft.MinecraftUWP_8wekyb3d8bbwe!App
	*/
	std::wstring GetMinecraftApplicationId();

	// Launches an application given the ApplicationId and returns the process ID via pdwProcessId
	HRESULT LaunchApplication(LPCWSTR ApplicationId, PDWORD pdwProcessId);

	// Wrapper for IPackageExecutionStateChangeNotification
	class StateChangeMonitor : public WRL::RuntimeClass<WRL::RuntimeClassFlags<WRL::ClassicCom>, IPackageExecutionStateChangeNotification> {
	public:
		StateChangeMonitor();

		virtual ~StateChangeMonitor();

		void initialize(const wchar_t *PackageId, ATL::CComPtr<IPackageDebugSettings> packageDebugSettingsIn);
		HRESULT __stdcall OnStateChanged(const wchar_t *PackageId, PACKAGE_EXECUTION_STATE state);
		PACKAGE_EXECUTION_STATE getState();
		void setCallback(void* CallbackPtr);

	private:
		void showStateMessageBox(PACKAGE_EXECUTION_STATE state);

		typedef void(*callbackfn)(PACKAGE_EXECUTION_STATE);
		
		callbackfn callbackPtr;
		PACKAGE_EXECUTION_STATE currentState;
		ATL::CComPtr<IPackageDebugSettings> packageDebugSettings;
		DWORD registrationId;
	};

	// Wrapper for IPackageDebugSettings
	class AppDebugger
	{
	public:
		AppDebugger(const std::wstring& PackageId);
		virtual ~AppDebugger();

		HRESULT GetHRESULT();

		void DisableDebugging();
		void EnableDebugging(std::wstring commandLineParameters);
		void Suspend();
		void Resume();
		void TerminateAllProcesses();

		PACKAGE_EXECUTION_STATE GetPackageExecutionState();
		void setStateChangeCallback(void* callbackPtr);
		void stopListening();

	private:
		std::wstring PackageFullName;
		HRESULT hResult;
		ATL::CComPtr<IPackageDebugSettings> debugSettings;
		WRL::ComPtr<StateChangeMonitor> stateChangeMonitor;
	};
};

#endif // __UTILS_H_
#pragma once
