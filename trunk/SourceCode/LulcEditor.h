#pragma once
#include "resource.h"
#include "Lulctree.h"
#include "afxwin.h"
//#include "EnvDoc.h"


#include <EasySize.h>
#include "afxcolorbutton.h"


// LulcEditor dialog

class LulcEditor : public CDialog
{
   DECLARE_EASYSIZE
   DECLARE_DYNAMIC(LulcEditor)

public:
	LulcEditor(CWnd* pParent = NULL);   // standard constructor
	virtual ~LulcEditor();

// Dialog Data
	enum { IDD = IDD_LULCEDITOR };

protected:
	LulcTree  *m_pLulcTree;
   CButton    m_expandButton;
   CTreeCtrl  m_treeCtrl;
   bool       m_expanded;
   CRect      m_windowRect;
   CEdit      m_label;
   CEdit      m_attrValue;

   bool       m_isDirty;

   void AddToCtrl( LulcNodeArray *pChildArray,  HTREEITEM hParent );
	HTREEITEM FindItem( HTREEITEM hItem, int level, int id );
	bool TestNode( LulcNode *pNode, int level, int id ){ return pNode->m_id == id && pNode->GetNodeLevel() == level;}
	void CollapseAll( HTREEITEM hItem );
   void ExpandAll( HTREEITEM hItem );

public:
   CString m_path;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
public:
   virtual BOOL OnInitDialog();
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnNMClickLulctreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnTvnSelchangedLulctreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnBnClickedExpand();
protected:
   virtual void OnCancel();
   virtual void OnOK();
public:
   afx_msg void OnBnClickedAdd();
   afx_msg void OnBnClickedRemove();
   afx_msg void OnBnClickedExport();
   afx_msg void OnEnChangeLabel();
   afx_msg void OnEnChangeId();
   };
