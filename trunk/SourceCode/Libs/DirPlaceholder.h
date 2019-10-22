#pragma once
#include "libs.h"

#ifndef NO_MFC
#include <direct.h>
#endif

class LIBSAPI DirPlaceholder
   {
   public:
      DirPlaceholder(void);
      ~DirPlaceholder(void);

   protected:
      TCHAR *m_cwd;
   };
