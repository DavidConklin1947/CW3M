
#include "libs.h"
#pragma hdrstop

#include "misc.h"
#ifndef NO_MFC
#include <afxtempl.h>
#endif

void GetTileParams( int count, int &cols, CArray<int,int> &rowArray )
   {
   cols = 0;

   if ( count == 0 )
      return;
   
   while( count > cols * (cols+1) )
      cols++;

   rowArray.RemoveAll();
   rowArray.SetSize( cols );

   // have rows, now get cols
   for ( int i=0; i < cols; i++ )
      rowArray[ i ] = cols;  // start with this

   int changeCols = count - ( cols*cols );  // 0 for regular grid, positive if cols needs to increase, negative if cols needs to decrease

   for ( int i=0; i < abs( changeCols ); i++ )
      {
      if ( changeCols < 0 )
         rowArray[ i ] -= 1;
      else
         rowArray[ i ] += 1;
      }
   }



int CleanFileName( LPCTSTR filename)
   {
   LPTSTR ptr = (LPTSTR) filename;

   while (*ptr != NULL)
      {
      switch (*ptr)
         {
         case ' ':   *ptr = '_';    break;  // blanks
         case '\\':  *ptr = '_';    break;  // illegal filename characters 
         case '/':   *ptr = '_';    break;
         case ':':   *ptr = '_';    break;
         case '*':   *ptr = '_';    break;
         case '"':   *ptr = '_';    break;
         case '?':   *ptr = '_';    break;
         case '<':   *ptr = '_';    break;
         case '>':   *ptr = '_';    break;
         case '|':   *ptr = '_';    break;
         }
      ptr++;
      }

   //filename.ReleaseBuffer();
   return lstrlen(filename);
   }

 
int Tokenize( const TCHAR* str, const TCHAR* delimiters, CStringArray &tokens)
   {
   ASSERT( str && delimiters );

   int length = (int) _tcslen( str )+1;
 
   TCHAR *buffer = new TCHAR[ length ];
   _tcscpy_s( buffer, length, str );

   tokens.RemoveAll();
   
   TCHAR* pos = 0;
   TCHAR* token = _tcstok_s( buffer, delimiters, &pos ); // Get first token
   
   while( token != NULL )
      {
      tokens.Add( token );
 
      // Get next token, note that first parameter is NULL
      token = _tcstok_s( NULL, delimiters, &pos );
      }

   delete [] buffer;

   return (int) tokens.GetSize();
   }
