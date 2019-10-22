#include "stdafx.h"
#include "EnvException.h"

#pragma warning(suppress: 6031)

#ifndef NO_MFC
IMPLEMENT_DYNAMIC(EnvException, CException)
#endif

EnvException::EnvException() 
:  m_errorStr( "Unspecified Error" )
   {
   }

EnvException::EnvException( LPCSTR errorStr )
:  m_errorStr( errorStr ) 
   {
   }

EnvException::~EnvException()
   {
   }

const char * EnvException::what() const 
   { 
     return (const char*)m_errorStr.GetString() ; 
   }

BOOL EnvException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT /*pnHelpContext*/)
   {
#ifndef NO_MFC
   ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));
#endif
   if (lpszError == NULL || nMaxError == 0) 
      {
      return FALSE;
      }

   lstrcpyn(lpszError, m_errorStr, nMaxError);

   return TRUE;
   }
