#include "libs.h"
#pragma hdrstop

#include "DirPlaceholder.h"

#ifdef NO_MFC
#include<unistd.h>
#endif

DirPlaceholder::DirPlaceholder(void )
   {
#ifndef NO_MFC
   m_cwd = _getcwd( NULL, 0 );
#else
   const unsigned int MAXPATHLEN=2048;
   m_cwd= (TCHAR*)malloc(sizeof(TCHAR)*MAXPATHLEN);
   getcwd(m_cwd,MAXPATHLEN);
#endif
   }

DirPlaceholder::~DirPlaceholder(void)
   {
#ifndef NO_MFC
   _chdir( m_cwd );
#else
   chdir(m_cwd);
#endif
   free( m_cwd );
   }
