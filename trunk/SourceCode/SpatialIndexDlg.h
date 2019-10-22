#pragma once
#include "afxcmn.h"

#include <map.h>

// SpatialIndexDlg dialog

class SpatialIndexDlg : public CDialog
{
	DECLARE_DYNAMIC(SpatialIndexDlg)

public:
	SpatialIndexDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~SpatialIndexDlg();

   static int MapProc( Map *pMap, NOTIFY_TYPE type, int a0, LONG_PTR a1, LONG_PTR extra );

// Dialog Data
	enum { IDD = IDD_SPATIALINDEX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   bool m_canceled;
   bool m_inBuild;

public:
   CProgressCtrl m_progress;
   
protected:
   virtual void OnOK();
   virtual void OnCancel();
public:
   int m_maxDistance;
   };
