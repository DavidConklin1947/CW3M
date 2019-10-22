#pragma once
#include "afxcmn.h"

#include "resource.h"

class MapLayer;
class QNode;


// QueryViewer dialog

class QueryViewer : public CDialog
{
	DECLARE_DYNAMIC(QueryViewer)

public:
	QueryViewer(LPCTSTR query, MapLayer *pLayer, CWnd* pParent = NULL);   // standard constructor
	virtual ~QueryViewer();

// Dialog Data
	enum { IDD = IDD_QUERYVIEWER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

   MapLayer *m_pLayer;

public:
   CString m_query;
   CTreeCtrl m_tree;
protected:
   virtual void OnOK();
   
   void AddNode( HTREEITEM hParent, QNode *pNode );

   };
