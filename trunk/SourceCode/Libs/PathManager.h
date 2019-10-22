#pragma once
#include "libs.h"

#include "Path.h"

using namespace nsPath;


class LIBSAPI PathManager
   {
   public:
      PathManager(void);
      ~PathManager(void);

   protected:
      static CStringArray m_pathArray;

      static int SearchPaths( CPath &path, FILE **fp, LPCTSTR mode ); // Last In, First out

   public:
      static LPCTSTR GetPath(int i);    // this will always be terminated with a '\'
      static int AddPath( LPCTSTR path );
      static int SetPath( int index, LPCTSTR path );
      static int FindPath( LPCTSTR path, CString &fullPath );        // finds first full path that contains passed in path if relative or filename, return value: > 0 = success; < 0 = failure (file not found), 0 = path fully qualified and found 
      static CString MakeAbsolutePath(CString origPath, int pathManagerIndex);
      static int FileOpen( LPCTSTR path, FILE **fp, LPCTSTR mode );
   };


