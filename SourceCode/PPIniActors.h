#pragma once

#include "resource.h"

#include <TabPageSSL.h>
#include "afxwin.h"

#include <grid\gridctrl.h>


class IniFileEditor;
class PPIniActors;


class ValueGrid : public CGridCtrl
{
public:
   PPIniActors *m_pParent;

   //virtual void  EndEditing() { m_pParent->MakeDirty(); }
   //afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point) { m_pParent->MakeDirty(); }
};



// PPIniActors dialog

class PPIniActors : public CTabPageSSL
{
	DECLARE_DYNAMIC(PPIniActors)

public:
	PPIniActors( IniFileEditor *pParent);   // standard constructor
	virtual ~PPIniActors();

// Dialog Data
	enum { IDD = IDD_INIACTORS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
   void LoadActorValueGrid();

	DECLARE_MESSAGE_MAP()
public:
   IniFileEditor *m_pParent;

   int  m_decisionFreq;
   BOOL m_assocAttr;
   BOOL m_assocValues;
   BOOL m_assocNeighbors;
   BOOL m_assocNone;
   int  m_method;

   // actor decisionmaking
   BOOL m_useAltruism;
   BOOL m_useActorValues;
   BOOL m_usePolicyPreferences;
   BOOL m_useUtility;

   ValueGrid m_grid;

   bool StoreChanges( void );

   afx_msg void OnBnClickedEditactordb2();
   afx_msg void OnBnClickedEditactordb();
   afx_msg void MakeDirty();

   virtual BOOL OnInitDialog();
   CStatic m_placeHolder;
   };
