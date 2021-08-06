// dllmain.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include <afxwin.h>
#include <afxdllx.h>

#include <Flow\flow.h>
#include "HBV.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

static AFX_EXTENSION_MODULE HBVDLL = { NULL, NULL };

HBV *theModel = NULL;


extern "C" int APIENTRY
DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
	// Remove this if you use lpReserved
	UNREFERENCED_PARAMETER(lpReserved);

	if (dwReason == DLL_PROCESS_ATTACH)
	{
		TRACE0("HBV.DLL Initializing!\n");
		
		// Extension DLL one-time initialization
		if (!AfxInitExtensionModule(HBVDLL, hInstance))
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

		new CDynLinkLibrary(HBVDLL);

            /*** TODO:  instantiate any models/processes ***/
      ASSERT( theModel == NULL );
      theModel = new HBV;
      ASSERT( theModel != NULL );

	}
	else if (dwReason == DLL_PROCESS_DETACH)
	{
		TRACE0("HBV.DLL Terminating!\n");

         if ( theModel != NULL)
            delete theModel;

		// Terminate the library before destructors are called
		AfxTermExtensionModule(HBVDLL);
	}
	return 1;   // ok
}

extern "C" float PASCAL EXPORT HBVdailyProcess(FlowContext *pFlowContext);
//x extern "C" float PASCAL EXPORT HBVdailyProcess(FlowContext *pFlowContext);
extern "C" float PASCAL EXPORT InitHBV_Global( FlowContext *pFlowContext, LPCTSTR initStr );

extern "C" BOOL PASCAL EXPORT CalcDailyUrbanWater(FlowContext *pFlowContext); 
extern "C" BOOL PASCAL EXPORT CalcDailyUrbanWaterInit(FlowContext *pFlowContext); 
extern "C" BOOL PASCAL EXPORT CalcDailyUrbanWaterRun(FlowContext *pFlowContext); 

float PASCAL HBVdailyProcess(FlowContext *pFlowContext) { return theModel->HBVdailyProcess(pFlowContext); }
//x float PASCAL HBVdailyProcess(FlowContext *pFlowContext) { return theModel->HBVdailyProcess(pFlowContext); }
float PASCAL InitHBV_Global( FlowContext *pFlowContext, LPCTSTR initInfo ) { return theModel->InitHBV_Global( pFlowContext, initInfo ); }

BOOL PASCAL CalcDailyUrbanWater(FlowContext *pFlowContext) { return theModel->CalcDailyUrbanWater(pFlowContext); }
BOOL PASCAL CalcDailyUrbanWaterInit(FlowContext *pFlowContext) { return theModel->CalcDailyUrbanWaterInit(pFlowContext); }
BOOL PASCAL CalcDailyUrbanWaterRun(FlowContext *pFlowContext) { return theModel->CalcDailyUrbanWaterRun(pFlowContext); }

