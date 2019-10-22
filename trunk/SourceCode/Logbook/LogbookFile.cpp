// ==========================================================================
// LogbookFile.cpp
//
// Author : Marquet Mike
//
// Company : /
//
// Date of creation  : 14/04/2003
// Last modification : 14/04/2003
// ==========================================================================

// ==========================================================================
// Les Includes
// ==========================================================================

#include "stdafx.h"
#include <io.h>
#include <share.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "Logbook.h"
#include "LogbookFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// class CLogbookFile

CLogbookFile::CLogbookFile(CLogbook *pLogbook, LPCTSTR lpszPathName, LPCTSTR lpszFileName)
 {
  m_nFileHandle   = -1;
  m_nFlushCounter = 0;
  m_pLogbook      = pLogbook;
  m_strFileName   = lpszFileName;
  m_LogbookPathName = lpszPathName;

  memset(&m_stCurrentDate, 0, sizeof(m_stCurrentDate));
 }

// --------------------------------------------------------------------------

CLogbookFile::~CLogbookFile()
 {
  if (m_nFileHandle != -1) _close(m_nFileHandle);

  m_nFileHandle = -1;
 }

// --------------------------------------------------------------------------
// PROTECTED MEMBER FUNCTIONS
// --------------------------------------------------------------------------

CString CLogbookFile::GetRenameFileName()
 {
  CString str;
  
  int I = 0;

  while (I < 255)
   {
    str.Format("%s_%d.txt", "C:\\Envision", I++);

    DWORD dwValue = GetFileAttributes(str);

    if (dwValue == -1) return str;
   }

  return "";
 }

// --------------------------------------------------------------------------

CString CLogbookFile::GetStringType(LOGBOOK_TEXTTYPES nType)
 {
  CString str;

  switch(nType)
   {
    case LOGBOOK_INFO       : str = "INFO"; break;
    case LOGBOOK_WARNING    : str = "WARNING"; break;
    case LOGBOOK_ERROR      : str = "ERROR"; break;
    case LOGBOOK_FATAL      : str = "FATAL"; break;
    case LOGBOOK_SYSTEM     : str = "SYSTEM"; break;
    case LOGBOOK_DEBUG1     : str = "DEBUG1"; break;
    case LOGBOOK_DEBUG2     : str = "DEBUG2"; break;
    case LOGBOOK_DEBUG3     : str = "DEBUG3"; break;
    case LOGBOOK_DEBUG4     : str = "DEBUG4"; break;
    case LOGBOOK_TRACE      : str = "TRACE"; break;
    case LOGBOOK_PERSONAL1  : str = m_pLogbook->m_strPersonal[0]; break;
    case LOGBOOK_PERSONAL2  : str = m_pLogbook->m_strPersonal[1]; break;
    case LOGBOOK_PERSONAL3  : str = m_pLogbook->m_strPersonal[2]; break;
    case LOGBOOK_PERSONAL4  : str = m_pLogbook->m_strPersonal[3]; break;
    case LOGBOOK_PERSONAL5  : str = m_pLogbook->m_strPersonal[4]; break;
    case LOGBOOK_PERSONAL6  : str = m_pLogbook->m_strPersonal[5]; break;
    case LOGBOOK_PERSONAL7  : str = m_pLogbook->m_strPersonal[6]; break;
    case LOGBOOK_PERSONAL8  : str = m_pLogbook->m_strPersonal[7]; break;
    case LOGBOOK_PERSONAL9  : str = m_pLogbook->m_strPersonal[8]; break;
    case LOGBOOK_PERSONAL10 : str = m_pLogbook->m_strPersonal[9]; break;
    case LOGBOOK_PERSONAL11 : str = m_pLogbook->m_strPersonal[10]; break;
    case LOGBOOK_PERSONAL12 : str = m_pLogbook->m_strPersonal[11]; break;
    case LOGBOOK_PERSONAL13 : str = m_pLogbook->m_strPersonal[12]; break;
    case LOGBOOK_PERSONAL14 : str = m_pLogbook->m_strPersonal[13]; break;
   }

  int nMaxLen = 15;

  for (int I=str.GetLength(); I<nMaxLen; I++)
   {
    str += " ";
   }

  if (str.GetLength() > 15) str.SetAt(nMaxLen, '\0');

  return str;
 }

// --------------------------------------------------------------------------

void CLogbookFile::MakeSeparator(char *sz, char cStyle, int nLen)
   {
   int i;
   for (i=0; i<nLen; i++) sz[i] = cStyle;

   sz[i++] = '\n';
   sz[i  ] = '\0';
   }

// --------------------------------------------------------------------------

BOOL CLogbookFile::Open(BOOL bTrunc)
 {
  if (m_nFileHandle != -1) // File is already opened
   {
    if ( !GetFileInfos()->bCreateNewFileEachDay ) return TRUE;

    SYSTEMTIME stCurrentDate;

    GetLocalTime(&stCurrentDate);

    if (m_stCurrentDate.wDay   == stCurrentDate.wDay   &&
        m_stCurrentDate.wMonth == stCurrentDate.wMonth &&
        m_stCurrentDate.wYear  == stCurrentDate.wYear     ) return TRUE;

    memcpy(&m_stCurrentDate, &stCurrentDate, sizeof(SYSTEMTIME));
    
    _close(m_nFileHandle);

    m_nFileHandle = -1;
   }

  int nFlags = _O_APPEND | _O_CREAT | _O_TEXT | _O_WRONLY;

  if ( bTrunc || GetFileInfos()->bClearFileEachOpen ) nFlags |= _O_TRUNC;

  CString strFileName = m_strFileName;

  if ( GetFileInfos()->bCreateNewFileEachDay )
   {
    SYSTEMTIME stST;

    GetLocalTime(&stST);

    strFileName.Format("%04u%02u%02u_%s", stST.wYear, stST.wMonth, stST.wDay, m_strFileName);
   }

  CString strPathName = strFileName;
  PWSTR userPath;
  SHGetKnownFolderPath(FOLDERID_Documents, 0, NULL, &userPath);
  char szBuffer[255];
  WideCharToMultiByte(CP_ACP, 0, userPath, -1, szBuffer, sizeof(szBuffer), NULL, NULL);
  strPathName.Format("%s\\%s", szBuffer, strFileName);

  errno_t err_local = _sopen_s( &m_nFileHandle, strPathName, nFlags, _SH_DENYNO, _S_IREAD | _S_IWRITE );
  int err_int = (int)err_local;
  CString msg; msg.Format("strPathName = %s; err_int = %d\n", strPathName, err_int);
  OutputDebugString(msg);

  //if (m_nFileHandle == -1)
  if (err_int != 0)
     {
     return FALSE;
     }
  else
     {
     // m_LogbookPathName.Format("%s", strPathName);
     return TRUE;
     }
 } // end of Open()


// --------------------------------------------------------------------------
// PUBLIC MEMBER FUNCTIONS
// --------------------------------------------------------------------------

int CLogbookFile::AddLine(const LOGBOOKITEM_INFO &stInfo, const SYSTEMTIME &stST, BOOL bDisplayTime, BOOL bDisplayType)
 {
  // try to open the file (if not already opened)
  if ( !Open() ) return -1;

  if ( GetFileInfos()->nMaxSize )
   {
    // Verify file size and if file size is greater than max allowed size, close
    // file, rename it and open new one.
    ULONG nFileSize = (ULONG)_lseek(m_nFileHandle, 0, SEEK_END);

    int nTextLen = stInfo.strText.GetLength();

    if (stInfo.nTextType == LOGBOOK_NONE) nTextLen = 255; // is separator line

    if ( (nFileSize + nTextLen) > GetFileInfos()->nMaxSize )
     {
      _close(m_nFileHandle);

      m_nFileHandle = -1;
      /*
      if ( !GetFileInfos()->bReuseSameFile )
       {
        CString strNewFileName = GetRenameFileName();

        if ( !strNewFileName.IsEmpty() ) CFile::Rename(m_LogbookPathName, strNewFileName);
       }
*/
      if ( !Open(GetFileInfos()->bReuseSameFile) || !IsOpen() ) return FALSE;
     }
   }

  if ( GetFileInfos()->nMaxFlushCounter && m_nFlushCounter > GetFileInfos()->nMaxFlushCounter )
   {
    m_nFlushCounter = 0;

    _commit(m_nFileHandle);
   }

  m_nFlushCounter++;

  if (stInfo.nTextType != LOGBOOK_NONE)
   {
    char *szText = new char [ stInfo.strText.GetLength() + 1 ];

    lstrcpy(szText, stInfo.strText);

    BOOL  bFirstLine = TRUE;
    char *next;
    char *sz = strtok_s(szText, "\n", &next );

    CString str;

    while (sz)
     {
      if (!bFirstLine)
       {
        bDisplayTime = FALSE;
        bDisplayType = FALSE;
       }
     
      str.Format("%19s | %s | %s\n",
                 bDisplayTime ? m_pLogbook->GetStringTime(stST,TRUE) : " ",
                 GetStringType(bDisplayType ? stInfo.nTextType : LOGBOOK_NONE),
                 sz);

      _write(m_nFileHandle, str, str.GetLength());

      sz = strtok_s(NULL, "\n", &next);

      bFirstLine = FALSE;
     }

    delete [] szText;
   }
  else {
        char szSeparator[255] = "\n";
        
        switch(stInfo.nSeparatorType)
         {
          case LOGBOOK_DBLSEPARATOR  : MakeSeparator(szSeparator, '='); break;
          case LOGBOOK_DASHSEPARATOR : MakeSeparator(szSeparator, '/'); break;
          case LOGBOOK_DOTSEPARATOR  : MakeSeparator(szSeparator, '.'); break;

          default : MakeSeparator(szSeparator, '-'); break;
         }

        _write(m_nFileHandle, szSeparator, lstrlen(szSeparator));
       }

  _commit(m_nFileHandle);

  return 1;
 }

// --------------------------------------------------------------------------
