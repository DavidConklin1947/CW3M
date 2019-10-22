// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>

#include <EnvEngine\EnvContext.h>
#include "Modeler.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


/**** TODO - indicate model/process instance pointer ****/
Modeler *theModeler = NULL;

static AFX_EXTENSION_MODULE ModelerDLL = { NULL, NULL };

extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
      /**** TODO: update trace string with module name ****/
		TRACE0("Modeler.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
      /**** TODO: update AfxInitExtensionModule with module name ****/
		if (!AfxInitExtensionModule(ModelerDLL, hInstance))
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

		new CDynLinkLibrary(ModelerDLL);
     
      ASSERT( theModeler == NULL );
      theModeler = new Modeler;
      ASSERT( theModeler != NULL );
	}
    
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("Modeler.DLL Terminating!\n");

      if ( theModeler != NULL)
         {
         delete theModeler;
         theModeler = NULL;
         }
		// Terminate the library before destructors are called
		AfxTermExtensionModule(ModelerDLL);
	}
	return 1;   // ok
}


/**** TODO: Comment out any unneeded interfaces ****/
// for eval model
extern "C" BOOL PASCAL EXPORT EMInit( EnvContext *pContext, LPCTSTR initStr );
extern "C" BOOL PASCAL EXPORT EMInitRun( EnvContext *pContext, bool useInitialSeed );
extern "C" BOOL PASCAL EXPORT EMRun( EnvContext *pContext );
extern "C" BOOL PASCAL EXPORT EMSetup( EnvContext *pContext, HWND hWnd );
extern "C" int  PASCAL EXPORT EMInputVar( int modelID, MODEL_VAR** modelVar );
extern "C" int  PASCAL EXPORT EMOutputVar( int modelID, MODEL_VAR** modelVar );

// for autonomous processes
extern "C" BOOL PASCAL EXPORT APInit( EnvContext *pEnvContext, LPCTSTR initStr );
extern "C" BOOL PASCAL EXPORT APInitRun( EnvContext *pEnvContext, bool useInitialSeed );
extern "C" BOOL PASCAL EXPORT APRun( EnvContext *pEnvContext );
extern "C" BOOL PASCAL EXPORT APSetup( EnvContext *pContext, HWND hWnd );
extern "C" int  PASCAL EXPORT APInputVar( int modelID, MODEL_VAR** modelVar );
extern "C" int  PASCAL EXPORT APOutputVar( int modelID, MODEL_VAR** modelVar );
//extern "C" BOOL PASCAL EXPORT APProcessMap( MapLayer *pLayer, int id );


/////////////////////////////////////////////////////////////////////////////////////
///////////               A U T O N O M O U S    P R O C E S S          /////////////
/////////////////////////////////////////////////////////////////////////////////////


BOOL PASCAL APInit( EnvContext *pEnvContext, LPCTSTR initStr )       { return theModeler->ApInit( pEnvContext, initStr ); }
BOOL PASCAL APInitRun( EnvContext *pEnvContext, bool useInitSeed )   { return theModeler->ApInitRun( pEnvContext, useInitSeed ); }
BOOL PASCAL APRun( EnvContext *pEnvContext )                         { return theModeler->ApRun( pEnvContext );  }
BOOL PASCAL APSetup( EnvContext *pContext, HWND hWnd )      { return theModeler->ApSetup( pContext, hWnd ); }
int  PASCAL APInputVar( int id, MODEL_VAR** modelVar )               { return theModeler->ApInputVar( id, modelVar ); }
int  PASCAL APOutputVar( int id, MODEL_VAR** modelVar )              { return theModeler->ApOutputVar( id, modelVar ); }
//BOOL PASCAL APProcessMap( MapLayer *pLayer, int id )              { return theModeler->ApProcessMap( pLayer, id ); }


/////////////////////////////////////////////////////////////////////////////////////
//////////////      E V A L     M O D E L            ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////
BOOL PASCAL EMInit( EnvContext *pEnvContext, LPCTSTR initStr )    { return theModeler->EmInit( pEnvContext, initStr ); }
BOOL PASCAL EMInitRun( EnvContext *pEnvContext, bool useInitSeed ){ return theModeler->EmInitRun( pEnvContext, useInitSeed ); }
BOOL PASCAL EMRun( EnvContext *pEnvContext )                      { return theModeler->EmRun( pEnvContext );  }
BOOL PASCAL EMSetup( EnvContext *pContext, HWND hWnd )   { return theModeler->EmSetup( pContext, hWnd ); }
int  PASCAL EMInputVar( int id, MODEL_VAR** modelVar )            { return theModeler->EmInputVar( id, modelVar ); }
int  PASCAL EMOutputVar( int id, MODEL_VAR** modelVar )           { return theModeler->EmOutputVar( id, modelVar ); }

