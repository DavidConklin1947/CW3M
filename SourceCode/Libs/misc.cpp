
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


void ApplySubstituteStrings(CString& str, PtrArray<SubstituteString> substitutes)
{
   if (str.Find('{') < 0) return;

   // iterate through the substitute stringss, applying substitutions as appropriate
   for (int i = 0; i < (int)substitutes.GetSize(); i++)
   {
      SubstituteString* pSubstitute = substitutes.GetAt(i);
      CString string_to_replace = pSubstitute->m_stringToReplace;
      if (str.Find(string_to_replace) < 0) continue;
      CString replacement_string = pSubstitute->m_replacementString;
      str.Replace(string_to_replace, replacement_string);
   }
} // end of ApplySubstituteStrings()


bool close_enough(double x, double y, double allowable_relative_diff, double allowable_near_zero)
{
   if (x == 0 && y == 0) return(true);

   double abs_x = fabs(x);
   double abs_y = fabs(y);

   double divisor = abs_x > abs_y ? abs_x : abs_y;
   double abs_relative_diff = fabs((x - y) / divisor);
   if (abs_relative_diff <= allowable_relative_diff) return(true);
   if (abs_x <= allowable_near_zero && abs_y <= allowable_near_zero) return(true);
   return(false);
} // end of close_enough(x, y, rel, abs)


bool close_enough_to_zero(double x, double allowable_near_zero) // returns true iff abs(x) <= abs(allowable_near_zero)
{
   ASSERT(allowable_near_zero >= 0);
   if (abs(x) <= abs(allowable_near_zero))
      return(true);
   else
      return(false);
} // end of close_enough_to_zero()

