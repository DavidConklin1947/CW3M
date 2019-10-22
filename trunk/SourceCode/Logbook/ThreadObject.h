// ==========================================================================
// ThreadObject.h
//
// Author : Marquet Mike
//
// Company : /
//
// Date of creation  : 14/04/2003
// Last modification : 14/04/2003
// ==========================================================================

#if !defined(AFX_THREADOBJECT_H__6AC375FE_7271_4B17_935F_98F9EBA517FC__INCLUDED_)
#define AFX_THREADOBJECT_H__6AC375FE_7271_4B17_935F_98F9EBA517FC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// ==========================================================================
// Les Includes
// ==========================================================================

#include "Logbook.h"

/////////////////////////////////////////////////////////////////////////////
// class CThreadObject

class CThreadObject : public CObject  
 {
  public :
           LOGBOOKITEM_INFO m_stItemInfo;
           SYSTEMTIME       m_stST;
           BOOL             m_bDisplayTime;
           BOOL             m_bDisplayType;

           CThreadObject();
           virtual ~CThreadObject();
 };

// ==========================================================================
// ==========================================================================

#endif // !defined(AFX_THREADOBJECT_H__6AC375FE_7271_4B17_935F_98F9EBA517FC__INCLUDED_)
