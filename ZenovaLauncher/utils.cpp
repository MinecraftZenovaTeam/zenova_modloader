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
