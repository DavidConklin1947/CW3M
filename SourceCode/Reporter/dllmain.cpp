// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>

#include <EnvEngine\EnvContext.h>
#include "Reporter.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/**** TODO - indicate model/process instance pointer ****/
Reporter *theReporter = NULL;

static AFX_EXTENSION_MODULE ReporterDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
      /**** TODO: update trace string with module name ****/
		TRACE0("Reporter.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
      /**** TODO: update AfxInitExtensionModule with module name ****/
		if (!AfxInitExtensionModule(ReporterDLL, hInstance))
			return 0;

		// Insert this DLL into the resource chain
		// NOTE: If this Extension DLL is being implicitly linked to by
		//  an MFC Regular DLL (such as an ActiveX Control)
		//  instead of an MFC application, then you will want to
		//  remove this line from DllMain and put it in a separate
		//  function exported from this Extension DLL.  The Regular DLL
		//  that uses this Extension DLL should then explicitly call that
		//  function to initialize this Extension DLL.  Otherwise,
		//  the CDynLinkLibrary object will not be attached to the
		//  Regular DLL's resource chain, and serious problems will
		//  result.

		new CDynLinkLibrary(ReporterDLL);
     
      ASSERT( theReporter == NULL );
      theReporter = new Reporter;
      ASSERT( theReporter != NULL );
	}
    
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Reporter.DLL Terminating!\n");

      if ( theReporter != NULL)
         {
         delete theReporter;
         theReporter = NULL;
         }
		// Terminate the library before destructors are called
		AfxTermExtensionModule(ReporterDLL);
	}
	return 1;   // ok
}


// for autonomous processes
extern "C" BOOL PASCAL EXPORT APInit( EnvContext *pEnvContext, LPCTSTR initStr );
extern "C" BOOL PASCAL EXPORT APInitRun( EnvContext *pEnvContext, bool useInitialSeed );
extern "C" BOOL PASCAL EXPORT APRun( EnvContext *pEnvContext );
extern "C" BOOL PASCAL EXPORT APEndRun( EnvContext *pEnvContext );
extern "C" BOOL PASCAL EXPORT APSetup( EnvContext *pContext, HWND hWnd );
extern "C" int  PASCAL EXPORT APInputVar( int modelID, MODEL_VAR** modelVar );
extern "C" int  PASCAL EXPORT APOutputVar( int modelID, MODEL_VAR** modelVar );


/////////////////////////////////////////////////////////////////////////////////////
///////////               A U T O N O M O U S    P R O C E S S          /////////////
/////////////////////////////////////////////////////////////////////////////////////


BOOL PASCAL APInit( EnvContext *pEnvContext, LPCTSTR initStr )       { return theReporter->Init( pEnvContext, initStr ); }
BOOL PASCAL APInitRun( EnvContext *pEnvContext, bool useInitSeed )   { return theReporter->InitRun( pEnvContext, useInitSeed ); }
BOOL PASCAL APRun( EnvContext *pEnvContext )                         { return theReporter->Run( pEnvContext );  }
BOOL PASCAL APEndRun( EnvContext *pEnvContext )                      { return theReporter->EndRun( pEnvContext );  }
BOOL PASCAL APSetup( EnvContext *pContext, HWND hWnd )               { return theReporter->Setup( pContext, hWnd ); }
int  PASCAL APInputVar( int id, MODEL_VAR** modelVar )               { return theReporter->InputVar( id, modelVar ); }
int  PASCAL APOutputVar( int id, MODEL_VAR** modelVar )              { return theReporter->OutputVar( id, modelVar ); }
