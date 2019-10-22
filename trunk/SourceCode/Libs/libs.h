//#define LIBSAPI __declspec( dllexport )
#pragma once

#ifdef _WINDOWS

#ifdef BUILD_LIBS
#define LIBSAPI __declspec( dllexport )
#else 
#define LIBSAPI __declspec( dllimport )
#endif


#ifndef VC_EXTRALEAN
#define VC_EXTRALEAN            // Exclude rarely-used stuff from Windows headers
#endif

#include "..\targetver.h"

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS      // some CString constructors will be explicit

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxole.h>         // MFC OLE classes
#include <afxodlgs.h>       // MFC OLE dialog classes
#include <afxdisp.h>        // MFC Automation classes
#endif // _AFX_NO_OLE_SUPPORT

#ifndef _AFX_NO_DB_SUPPORT
#include <afxdb.h>                      // MFC ODBC database classes
#endif // _AFX_NO_DB_SUPPORT

#ifndef _AFX_NO_DAO_SUPPORT
#ifndef _WIN64
#include <afxdao.h>                     // MFC DAO database classes
#endif
#endif // _AFX_NO_DAO_SUPPORT

#ifndef _AFX_NO_OLE_SUPPORT
#include <afxdtctl.h>           // MFC support for Internet Explorer 4 Common Controls
#endif
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>                     // MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <afxadv.h>  // for CSharedFile (Clipboard stuff)
#include <afxdlgs.h>
#include <afxcontrolbars.h>

#else
//empty
#define LIBSAPI

#ifdef NO_MFC
#include <no_mfc_files/NoMfcFiles.h>
#endif

#endif // _WINDOWS

#include "stdstring.h"

