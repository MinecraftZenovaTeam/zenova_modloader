#ifndef __UTILS_H_
#define __UTILS_H_

#include <iostream>

namespace Util
{
	/* Converts a C string to a wchar_t* */
	wchar_t* CHARPTR_TO_WCHAR(const char* charptr);
	/* Converts a wstring to a C++ string */
	std::string WSTRING_TO_STRING(const std::wstring&);
	/* Get the path to the Minecraft local AppData path */
	std::string GetMinecraftAppDataPath();
};

#endif // __UTILS_H_
