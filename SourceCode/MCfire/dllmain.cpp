// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>

#include <EnvEngine\EnvContext.h>

#include "MCfire.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif


static AFX_EXTENSION_MODULE WWMfireDLL = { NULL, NULL };

WWMfire * theModel = NULL;


extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
      /**** TODO: update trace string with module name ****/
		TRACE0("WWMfire.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
      /**** TODO: update AfxInitExtensionModule with module name ****/
		if (!AfxInitExtensionModule(WWMfireDLL, hInstance))
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

      /**** TODO: update CDynLinkLibrary constructor with module name ****/
		new CDynLinkLibrary(WWMfireDLL);
     
      ASSERT( theModel == NULL );
      theModel = new WWMfire;
      ASSERT( theModel != NULL );

 	}
    
	else if (dwReason == DLL_PROCESS_DETACH)
	{
      /**** TODO: Update module name in trace string ****/
		TRACE0("WWMfire.DLL Terminating!\n");

      /**** TODO: Delete any instantiated models ****/
      if ( theModel != NULL)
         delete theModel;

		// Terminate the library before destructors are called
      /**** TODO: Update module name in AfxTermExtensionModule ****/
		AfxTermExtensionModule(WWMfireDLL);
	}
	return 1;   // ok
}

extern "C" bool PASCAL EXPORT WWMfire_DailyProcess(FlowContext *pFlowContext);
bool PASCAL WWMfire_DailyProcess(FlowContext *pFlowContext) { return theModel->WWMfire_DailyProcess(pFlowContext); }

