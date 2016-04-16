#ifndef __PROCESSUTILS_H_
#define __PROCESSUTILS_H_

namespace ProcessUtils
{
	// Windows Internal Functions
	typedef LONG(NTAPI *NtSuspendProcess)(IN HANDLE ProcessHandle);
	typedef LONG(NTAPI *NtResumeProcess)(IN HANDLE ProcessHandle);
	typedef LONG(NTAPI *NtTerminateProcess)(IN HANDLE ProcessHandle);

	void SuspendProcess(DWORD processId);
	void ResumeProcess(DWORD processId);
	void TerminateProcess(DWORD processId);

	/* Gets the ID of a process given its name */
	DWORD GetProcessId(const std::wstring& processName);
};

#endif // __PROCESSUTILS_H_
