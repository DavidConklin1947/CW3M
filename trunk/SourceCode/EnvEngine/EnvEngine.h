// EnvEngine.h : main header file for the EnvEngine DLL
//

#pragma once

#ifndef NO_MFC
#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols
#endif

#include <MAP.h>
#include "EnvModel.h"

#ifndef NO_MFC
// CEnvEngineApp
// See EnvEngine.cpp for the implementation of this class
//

class CEnvEngineApp : public CWinApp
{
public:
	CEnvEngineApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()

protected:
   EnvModel     m_model;

   Map         *m_pMap;
   DataManager *m_pDataManager;
   QueryEngine *m_pQueryEngine;   
   
  };



// interface functions
extern "C"
{
int PASCAL InitEngine( LPCTSTR envxFile, Map *pMap, int initFlags );

}
#endif
