// This MFC Samples source code demonstrates using MFC Microsoft Office Fluent User Interface 
// (the "Fluent UI") and is provided only as referential material to supplement the 
// Microsoft Foundation Classes Reference and related electronic documentation 
// included with the MFC C++ library software.  
// License terms to copy, use or distribute the Fluent UI are available separately.  
// To learn more about our Fluent UI licensing program, please visit 
// http://msdn.microsoft.com/officeui.
//
// Copyright (C) Microsoft Corporation
// All rights reserved.

// Envision.h : main header file for the Envision application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols

class EnvCommandLineInfo;


// CEnvApp:
// See Envision.cpp for the implementation of this class
//

class CEnvApp : public CWinAppEx
{
public:
	CEnvApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	UINT  m_nAppLook;
 	BOOL  m_bHiColorIcons;
	virtual void PreLoadState();
	virtual void LoadCustomState();
	virtual void SaveCustomState();

   bool ProcessEnvCmdLineArgs( EnvCommandLineInfo & );

	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()

public:
   afx_msg void OnHelpUpdateEnvision();
   };

extern CEnvApp theApp;
