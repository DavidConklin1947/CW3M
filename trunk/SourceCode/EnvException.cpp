#include "stdafx.h"
#include ".\envexception.h"

IMPLEMENT_DYNAMIC(EnvException, CException)

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
   return (const char*)LPCSTR( m_errorStr ); 
   }

BOOL EnvException::GetErrorMessage(LPTSTR lpszError, UINT nMaxError, PUINT /*pnHelpContext*/)
   {
   ASSERT(lpszError != NULL && AfxIsValidString(lpszError, nMaxError));

   if (lpszError == NULL || nMaxError == 0) 
      {
      return FALSE;
      }

   lstrcpyn(lpszError, m_errorStr, nMaxError);

   return TRUE;
   }