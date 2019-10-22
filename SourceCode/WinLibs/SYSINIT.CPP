//#if defined _MFC
#include "winlibs.h"
//#else
//#include <windowsb.h>
//#endif

#pragma hdrstop


#include <sysinit.hpp>
#include <colors.hpp>


//-------------------- system-wide constants ----------------------//

HMODULE ghSysResDLL = 0;

COLORREF gColorRef[ 16 ] =
   {
   BLACK, BLUE,   GREEN,   CYAN,   RED,   MAGENTA,   BROWN,  LTGRAY, \
   GRAY,  LTBLUE, LTGREEN, LTCYAN, LTRED, LTMAGENTA, YELLOW, WHITE
   };

#if !defined _MFC

HINSTANCE ghInst = 0;


//----------------------- Library initialization ------------------//

BOOL InitSysLibApp( void )
   {
   return TRUE;
   }


BOOL InitSysLibInst( HINSTANCE hInst )
   {
   //-- load Res DLL --//
   ghSysResDLL = LoadLibrary( (LPSTR) "SYSRES.DLL" );

   if ( (UINT_PTR) ghSysResDLL < 32 )    // error occurred?
      {
      MessageBox( NULL, "Couldn't find SYSRES.DLL", "Warning", MB_OK );
      ghSysResDLL = (HMODULE) 0;
      }

   ghInst = hInst;

   return TRUE;
   }




//---------------------- Library shutdown -------------------------//

BOOL CloseSysLib( void )
   {
   if ( ghSysResDLL )
      FreeLibrary( ghSysResDLL );

   return TRUE;
   }


#endif //!defined _MFC
