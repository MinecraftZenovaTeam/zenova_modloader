#ifndef __THREADWORKER_H_
#define __THREADWORKER_H_

#include <iostream>

namespace ThreadWorker
{
	/* thread states */
	enum ThreadState
	{
		THREAD_SUSPENDED,
		THREAD_RUNNING
	};

	/* Returns the name of the module that owns the given thread */
	std::wstring GetThreadOwnerModule(DWORD dwProcId, HANDLE hThread, std::wstring& wstrModulePath, PDWORD_PTR pModuleStartAddr, PDWORD_PTR pThreadStartAddr);
	/* Either suspends or resumes all threads in the given module */
	BOOL SetModuleThreadState(const DWORD dwProcessId, const std::wstring ModuleName, BOOL _threadStatus);
};

#endif // __THREADWORKER_H_
