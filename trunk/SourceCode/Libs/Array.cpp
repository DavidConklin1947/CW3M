#include "libs.h"
#pragma hdrstop

#include "ARRAY.HPP"


Array::Array()
   {
   //-- initialize error handler --//
   //void (*(Array::lpFnErrorHandler))( int ) = DefaultHandler;
   lpFnErrorHandler = DefaultHandler;

   //-- class-global constant initialization --//
   AllocIncr = 256;   // 128 bytes per allocation 
   }

//-- default exception handler --//
void DefaultHandler( int err )
   {
   switch (err)
     {
     case AE_ALLOC :
        //error_msg("memory allocation failure");
        break;
     case AE_TOO_LONG :
        //error_msg( "maximum size exceeded" );
     break;
     }
   return;
   }


