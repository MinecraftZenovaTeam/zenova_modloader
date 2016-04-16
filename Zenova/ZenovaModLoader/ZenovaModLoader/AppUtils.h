#ifndef __APPUTILS_H_
#define __APPUTILS_H_

/*
 * Features basic tools for dealing with Win10 apps and a wrapper for IPackageDebugSettings
 */

#include <Windows.h>
#include <iostream>
#include <string>
#include <ShObjIdl.h>
#include <atlbase.h>

namespace AppUtils
{
	std::wstring GetMinecraftPackageId();
	std::wstring GetMinecraftApplicationId();

	class AppDebugger
	{
	public:
		AppDebugger(const std::wstring& packageFullName);

		HRESULT GetHRESULT();

		void DisableDebugging();
		void EnableDebugging(std::wstring commandLineParameters);
		void Suspend();
		void Resume();
		void TerminateAllProcesses();

		PACKAGE_EXECUTION_STATE GetPackageExecutionState();
		void PrintExecutionState();
	private:
		std::wstring PackageFullName;
		HRESULT hResult;

		ATL::CComQIPtr<IPackageDebugSettings> CreatePackageDebugger();
	};
};

#endif // __UTILS_H_
#pragma once
