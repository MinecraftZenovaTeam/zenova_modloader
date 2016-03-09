#include <Windows.h>
#include <shlobj.h>

#include "utils.h"

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

/* Get the path to the Minecraft local AppData path */
std::string Util::GetMinecraftAppDataPath()
{
	TCHAR szPath[MAX_PATH];
	if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, szPath)))
	{
		std::wstring appData(szPath);
		appData += L"\\Packages\\Microsoft.MinecraftUWP_8wekyb3d8bbwe\\LocalState\\games\\com.mojang\\";

		std::cout << "Found Minecraft AppData folder at: " << WSTRING_TO_STRING(appData).c_str() << "\n";

		return WSTRING_TO_STRING(appData);
	}
}
