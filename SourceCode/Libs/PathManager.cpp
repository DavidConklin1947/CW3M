#include "libs.h"
#pragma hdrstop

#include "PathManager.h"



CStringArray PathManager::m_pathArray;


PathManager::PathManager(void)
   {
   }


PathManager::~PathManager(void)
   {
   }


int PathManager::AddPath( LPCTSTR _path )
   {
   // clean up whatever is passed in
   CPath path( _path, epcTrim | epcSlashToBackslash );

   path.AddBackslash();
   
   return (int) m_pathArray.Add( (CString)path );
   }


int PathManager::SetPath( int index, LPCTSTR _path )
   {
   if ( index < 0 )
      return -1;
       
   if ( index >= (int) m_pathArray.GetSize() )
      return -2;

   // clean up whatever is passed in
   CPath path( _path, epcTrim | epcSlashToBackslash );
   path.AddBackslash();

   m_pathArray[ index ] = path;

   return index;
   }


LPCTSTR PathManager::GetPath(int i)
   {
   return m_pathArray[i]; 
   }       // this will always be terminated with a '\'


int PathManager::FindPath( LPCTSTR _path, CString &_fullPath )
   {
   _fullPath.Empty();

   //  return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
   CPath path( _path, epcTrim | epcSlashToBackslash );
   
   // case 1:  just a filename or a relative path - look through pathArray
   if ( path.IsFileSpec() || path.IsRelative() )  // only a file, not a path
      {
      int count = (int) m_pathArray.GetSize();
      for ( int i=count-1; i != 0; i-- )
         {
         CPath fullPath = m_pathArray.GetAt( i );

         // append the relative path that was passed in
         fullPath.Append( path );

         if ( fullPath.Exists() )
            {
            _fullPath = fullPath;
            return i+1;
            }
         }

      _fullPath.Empty();
      return -1;
      }

   else // it is already full qualified
      {
      CPath fullPath( path );
      if ( fullPath.Exists() )
         {
         _fullPath = path;
         return 0;
         }
      }

   _fullPath.Empty();
   return -2;
   }


int PathManager::FileOpen( LPCTSTR _path, FILE **fp, LPCTSTR _mode )
   {
   // _path can be:
   //   1) a file name  e.g. file.ext.    In this case, the paths are searched
   //         until a file is found.
   //   2) a relative path  e.g. somedir\file.ext.  In this case, search the paths 
   //   3) fully-qualified  e.g.  c:\somepath\file.ext. In that case, just open it
   //      if possible.
   //
   //  return value: > 0 = success; <= 0 = failure
   
   CPath path( _path, epcTrim | epcSlashToBackslash );

   LPCTSTR mode = _mode;
   if ( mode == NULL )
      mode = _T("rt");

   errno_t err = 0;
   *fp = NULL;

   // case 1:  just a filename
   if ( path.IsFileSpec() )  // only a file, not a path
      {
      PCTSTR file = (PCTSTR)path;
    	err = fopen_s( fp, file, mode );  // first, look in the current directory

      if ( ! err && fp != NULL )
         return 1;
      else                             // if not there, search paths
         return SearchPaths( path, fp, mode );
      }

   // case 2: relative path
   else if ( path.IsRelative() )
      {
      return SearchPaths( path, fp, mode );
      }

   // case 3:: fully qualified path
   else 
      {
      PCTSTR file = (PCTSTR)path;
      err = fopen_s( fp, file, mode );  // first, look in the current directory

      if ( ! err && fp != NULL )
         return 1;
      else                             // if not there, search paths
         return SearchPaths( path, fp, mode );
      }

   return 0;
   }


int PathManager::SearchPaths( CPath &path, FILE **fp, LPCTSTR mode )
   {
   errno_t err = 0;
   *fp = NULL;

   int count = (int) m_pathArray.GetSize();
   for ( int i=count-1; i != 0; i-- )
      {
      CPath fullPath = m_pathArray.GetAt( i );

      // append the relative path that was passed in
      fullPath.Append( path );

      // try to open it
      PCTSTR file = (PCTSTR)fullPath;
    	err = fopen_s( fp, file, mode );  // first, look in the current directory

      if ( ! err && fp != NULL )
         return 2;
      }

   return 0;
   }


CString PathManager::MakeAbsolutePath(CString origPath, int pathManagerIndex)
{
   CString rtn_str;
   char first_char = origPath.GetLength() < 1 ? ' ' : origPath.GetAt(0);
   if (first_char != '\\' && first_char != '/' && origPath.Find(':') < 0)
      rtn_str = PathManager::GetPath(pathManagerIndex) + origPath;
   else rtn_str = origPath;

   return(rtn_str);
} // end of MakeAbsolutePath()
