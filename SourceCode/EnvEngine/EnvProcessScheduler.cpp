#include "stdafx.h"
#include "EnvProcessScheduler.h"
//
#include "EnvModel.h"

std::map<ENV_PROCESS*, EnvProcessScheduler::ThreadData> EnvProcessScheduler::m_ThreadDataMap;
CCriticalSection EnvProcessScheduler::m_criticalSection;
int EnvProcessScheduler::m_maxThreadNum = omp_get_num_procs();

EnvProcessScheduler::EnvProcessScheduler(void)
{
}


EnvProcessScheduler::~EnvProcessScheduler(void)
{
}

void EnvProcessScheduler::init(PtrArray<ENV_PROCESS>& apInfoArray)
{
	m_ThreadDataMap.clear();
	int count = (int) apInfoArray.GetSize();
	
	for ( INT_PTR i=0; i < count; i++ )
	{
		ENV_PROCESS *pInfo = apInfoArray[ i ];
		ThreadData status;
		status.threadState = NOTSCHEDULED;
		status.id = pInfo;
		status.pEnvContext = NULL;
		m_ThreadDataMap.insert( std::make_pair(status.id, status) );
	}
}

void EnvProcessScheduler::signalThreadFinished( ENV_PROCESS* id )
{
	CSingleLock lock( &m_criticalSection );
	lock.Lock();
	
	ThreadDataMapIterator itr = m_ThreadDataMap.find( id );
	if( itr != m_ThreadDataMap.end() )
	{
		itr->second.threadState = FINISHED;
	}
	lock.Unlock();
}

UINT EnvProcessScheduler::workerThreadProc( LPVOID Param )
{
	EnvContext* pEnvContext = (EnvContext*)Param;
	ENV_PROCESS *pInfo = static_cast<ENV_PROCESS*>(pEnvContext->pExtensionInfo);
	pInfo->runFn( pEnvContext );
	signalThreadFinished( pInfo );
	return 0;
}

void EnvProcessScheduler::getRunnableProcesses(PtrArray<ENV_PROCESS>& apInfoArray, PtrArray<ENV_PROCESS>& runnableProcesses )
   {
   CSingleLock lock(&m_criticalSection);
   lock.Lock();
   ENV_PROCESS* pInfo = NULL;
   ThreadDataMapIterator itr;
   
   for (itr = m_ThreadDataMap.begin(); itr != m_ThreadDataMap.end(); itr++)
      {
      ThreadData status = itr->second;
      if (status.threadState == NOTSCHEDULED)
         {
         //get ENV_PROCESS			
         int count = (int)apInfoArray.GetSize();
         for (int j = 0; j < count; j++)
            {
            ENV_PROCESS *pInfoItr = apInfoArray[j];
            if (pInfoItr == status.id)
               {
               pInfo = pInfoItr;
               break;
               }
            }
   
         bool bAllDepsDone = true;
   
         // see if all its dependent are done.
         if (pInfo != NULL)
            {
            for (int j = 0; j < pInfo->dependencyCount; j++)
               {
               ENV_PROCESS* pDependentInfo = pInfo->dependencyArray[j];
               ThreadDataMapIterator itr1 = m_ThreadDataMap.find(pDependentInfo);
               if (itr1 != m_ThreadDataMap.end())
                  {
                  if (itr1->second.threadState != FINISHED)
                     {
                     bAllDepsDone = false;
                     break;
                     }
                  }
               }
            if (bAllDepsDone)
               {
               runnableProcesses.Add(pInfo);
               }
            }
         }
      }
   
   lock.Unlock();
   
   return;
   }


int EnvProcessScheduler::countActiveThreads()
   {
   CSingleLock lock(&m_criticalSection);
   lock.Lock();

   int count = 0;

   ThreadDataMapIterator itr;
   for (itr = m_ThreadDataMap.begin(); itr != m_ThreadDataMap.end(); itr++)
      {
      if (itr->second.threadState == RUNNING)
         {
         count++;
         }
      }
   lock.Unlock();

   return count;
}
