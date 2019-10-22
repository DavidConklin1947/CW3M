#pragma once

#ifndef NO_MFC
#include "PtrArray.h"
#include "EnvContext.h"
#include <map>
#include <omp.h>
#include <afxmt.h>

class EnvProcessScheduler
{
public:
	static int m_maxThreadNum;
	enum ThreadState
	{
		NOTSCHEDULED  =  0,
		RUNNING		  =  1,
		FINISHED      =  2
	};

	struct ThreadData
	{
		ENV_PROCESS* id;
		int threadState;
		EnvContext* pEnvContext;
	};

public:	
	static void init(PtrArray<ENV_PROCESS>& apInfoArray);
	static void signalThreadFinished( ENV_PROCESS* id );
	static UINT workerThreadProc( LPVOID Param );	
	static void getRunnableProcesses(PtrArray<ENV_PROCESS>& apInfoArray, PtrArray<ENV_PROCESS>& runnableProcesses );
	static int countActiveThreads();

public:
	static std::map<ENV_PROCESS*, ThreadData> m_ThreadDataMap;
	static CCriticalSection m_criticalSection;

public:
	EnvProcessScheduler(void);
	~EnvProcessScheduler(void);
};

typedef std::map<ENV_PROCESS*, EnvProcessScheduler::ThreadData>::iterator ThreadDataMapIterator;
#endif
