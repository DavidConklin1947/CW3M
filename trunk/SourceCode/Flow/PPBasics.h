#pragma once

#include "resource.h"

#include "Flow.h"

// PPBasics dialog

class PPBasics : public CPropertyPage
{
	DECLARE_DYNAMIC(PPBasics)

public:
	PPBasics( FlowModel *pFlow );
	virtual ~PPBasics();

// Dialog Data
	enum { IDD = IDD_BASICS };

   FlowModel *m_pFlow;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
   afx_msg void OnEnChangeInputpath();
   virtual BOOL OnInitDialog();
   };
