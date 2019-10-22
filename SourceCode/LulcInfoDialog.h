#pragma once

#include "resource.h"

#include "Lulctree.h"
#include "EnvDoc.h"
#include "afxwin.h"
#include "afxcmn.h"

// LulcInfoDialog dialog

class LulcInfoDialog : public CDialog
{
	DECLARE_DYNAMIC(LulcInfoDialog)

public:
	LulcInfoDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~LulcInfoDialog();

// Dialog Data
	enum { IDD = IDD_LULCINFO };

protected:
	LulcTree  *m_pLulcTree;
   CButton   m_expandButton;
   CEdit     m_lulcC;
   CEdit     m_lulcB;
   CEdit     m_lulcA;
   CTreeCtrl m_treeCtrl;
   bool      m_expanded;
   CRect     m_windowRect;

   void AddToCtrl( LulcNodeArray *pChildArray,  HTREEITEM hParent );
	HTREEITEM FindItem( HTREEITEM hItem, int level, int id );
	bool TestNode( LulcNode *pNode, int level, int id ){ return pNode->m_id == id && pNode->GetNodeLevel() == level;}
	void WriteParentsAbove( HTREEITEM hItem );
	void CollapseAll( HTREEITEM hItem );
   void ExpandAll( HTREEITEM hItem );

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
	
public:
   virtual BOOL OnInitDialog();
   afx_msg void OnSize(UINT nType, int cx, int cy);
   afx_msg void OnNMClickLulctreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnTvnSelchangedLulctreectrl(NMHDR *pNMHDR, LRESULT *pResult);
   afx_msg void OnEnChangeLulcc();
   afx_msg void OnEnChangeLulcb();
   afx_msg void OnEnChangeLulca();
   afx_msg void OnBnClickedExpand();
   virtual BOOL Create(CWnd* pParentWnd = NULL);
protected:
   virtual void OnCancel();
public:
   virtual BOOL DestroyWindow();
};
