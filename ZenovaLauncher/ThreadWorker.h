#ifndef __THREADWORKER_H_
#define __THREADWORKER_H_

#include <iostream>

/* Thread states */
#define THREAD_STATE_RUNNING TRUE
#define THREAD_STATE_SUSPENDED FALSE

namespace ThreadWorker
{
	/* Returns the name of the module that owns the given thread */
	std::wstring GetThreadOwnerModule(DWORD dwProcId, HANDLE hThread, std::wstring& wstrModulePath, PDWORD_PTR pModuleStartAddr, PDWORD_PTR pThreadStartAddr);
	/* Either suspends or resumes all threads in the given module */
	BOOL SetModuleThreadState(const DWORD dwProcessId, const std::wstring ModuleName, BOOL _threadStatus);
};

#endif // __THREADWORKER_H_
