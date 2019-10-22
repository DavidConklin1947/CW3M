#pragma once

#include "WinLib_resource.h"
#include "afxcmn.h"

#include "Maplayer.h"
//#include "DBTABLE.H"


// FieldSorterDlg dialog

class FieldSorterDlg : public CDialog
{
	DECLARE_DYNAMIC(FieldSorterDlg)

public:
	FieldSorterDlg(MapLayer *pLayer, CWnd* pParent = NULL);   // standard constructor
	virtual ~FieldSorterDlg();

// Dialog Data
	enum { IDD = IDD_LIBS_SORTCOLS };

   MapLayer *m_pMapLayer;
   //DbTable *m_pDbTable;


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   CDragListBox m_fields;
	virtual BOOL OnInitDialog();
   virtual void OnOK();
   };
