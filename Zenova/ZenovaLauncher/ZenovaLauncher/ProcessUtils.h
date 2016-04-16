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
};

#endif // __PROCESSUTILS_H_
