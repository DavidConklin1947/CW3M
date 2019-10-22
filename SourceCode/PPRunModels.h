#pragma once

#include "resource.h"
#include "afxwin.h"

// PPRunModels dialog

class PPRunModels : public CPropertyPage
{
	DECLARE_DYNAMIC(PPRunModels)

public:
	PPRunModels();
	virtual ~PPRunModels();

// Dialog Data
	enum { IDD = IDD_RUNMODELS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

protected:
   CCheckListBox m_models;

public:
   virtual BOOL OnInitDialog();
   int SetModelsToUse(void);
};
