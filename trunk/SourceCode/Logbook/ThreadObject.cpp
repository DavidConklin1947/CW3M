// ==========================================================================
// ThreadObject.cpp
//
// Author : Marquet Mike
//
// Company : /
//
// Date of creation  : 14/04/2003
// Last modification : 14/04/2003
// ==========================================================================

// ==========================================================================
// Les Includes
// ==========================================================================

#include "stdafx.h"
#include "ThreadObject.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// class CThreadObject

CThreadObject::CThreadObject()
 {
  CLogbook::LogbookItemCopy(NULL, m_stItemInfo);

  m_bDisplayTime = TRUE;
  m_bDisplayType = TRUE;

  memset(&m_stST, 0, sizeof(SYSTEMTIME));
 }

// --------------------------------------------------------------------------

CThreadObject::~CThreadObject()
 {
 }

// --------------------------------------------------------------------------
