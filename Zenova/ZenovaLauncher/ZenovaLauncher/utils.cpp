#include <Windows.h>
#include <ShlObj.h>

#include "utils.h"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

/* Converts a C string to a wchar_t* */
wchar_t* Util::CHARPTR_TO_WCHAR(const char* charptr)
{
	size_t size = strlen(charptr) + 1;
	size_t output;
	wchar_t* wa = new wchar_t[size];
	mbstowcs_s(&output, wa, size, charptr, size);
	return wa;
}

/* Converts a wstring to a C++ string */
std::string Util::WSTRING_TO_STRING(const std::wstring& oldstr)
{
	int len;
	int slength = (int)oldstr.length();
	len = WideCharToMultiByte(CP_ACP, 0, oldstr.c_str(), slength, 0, 0, 0, 0);
	std::string r(len, '\0');
	WideCharToMultiByte(CP_ACP, 0, oldstr.c_str(), slength, &r[0], len, 0, 0);
	return r;
}

/* Converts a standard string to a wide string */
std::wstring Util::STRING_TO_WSTRING(const std::string& oldstr)
{
	return std::wstring(oldstr.begin(), oldstr.end());
}

/* Get the path to the Minecraft local AppData path */
std::wstring Util::GetMinecraftAppDataPath()
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
	{
		std::wstring appData(szPath);
		appData += L"\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\LocalState\\games\\com.mojang\\";

		//std::cout << "Found Minecraft AppData folder at: " << WSTRING_TO_STRING(appData).c_str() << "\n";

		return appData;
	}
	return L"";
}

std::wstring Util::GetCommonFolder(KNOWNFOLDERID folderId)
{
	LPWSTR wszPath = NULL;
	
	if (SUCCEEDED(SHGetKnownFolderPath(folderId, KF_FLAG_DEFAULT_PATH, NULL, &wszPath)))
	{
		return wszPath;
	}
	return L"";
}

std::wstring Util::GetCurrentDirectory()
{
	/* // Gets the current directory from an EXE
	HMODULE hModule = GetModuleHandleW(NULL);
	WCHAR path[MAX_PATH];
	GetModuleFileNameW(hModule, path, MAX_PATH);
	*/

	// Gets the current directory from a DLL
	LPTSTR  path = new TCHAR[_MAX_PATH];
	GetModuleFileName((HINSTANCE)&__ImageBase, path, _MAX_PATH);

	std::wstring CurrentDirectory(path);
	CurrentDirectory = CurrentDirectory.substr(0, CurrentDirectory.rfind(L"\\")+1);
	
	return CurrentDirectory;
}